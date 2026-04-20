/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServer.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:51:23 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/20 15:07:23 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Socket.hpp"
#include "VServer.hpp"
#include "utils.hpp"

#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

VServer::VServer(): _reqHandler(this)
{
  _server_name = "";
  _root        = "";
  _setupFailed = false;
}

VServer::VServer(const VServerCfg& srvcfg): _reqHandler(this)
{
  _server_name = srvcfg.getServerName();
  _root        = srvcfg.getRoot();
  _cfg         = srvcfg;
  _setupFailed = false;
}

// FIXME: think about if assignment and stuff like this really make sense for my
// vserver classes
VServer::VServer(const VServer& o): _reqHandler(this)
{
  if (this != &o) {
    _server_name = o._server_name;
    _root        = o._root;
    _listen_fds  = o._listen_fds;

    _setupFailed = o._setupFailed;
    _cfg         = o._cfg;
  }
}

// NOTE: only for this use-case implemented the copy assignment constructor for
// RequestHandler. only place this is used: Webserv::_setupServers.
VServer& VServer::operator=(const VServer& o)
{
  if (this != &o) {
    _server_name = o._server_name;
    _root        = o._root;
    _listen_fds  = o._listen_fds;

    _setupFailed = o._setupFailed;
    _cfg         = o._cfg;
    _clients     = o._clients;
    _reqHandler  = RequestHandler(this);
  }
  return (*this);
}

// a little hack to avoid printing the "out of scope msg" for tmp servers not
// fully initialized
//
// FIXME: can this client removal go wrong? question arises bc _removeAllClients
// really deletes the Client object referenced by the pointers in _clients. But
// technically the cline-server relation is one-to-one, no?
VServer::~VServer()
{
  if (!_listen_fds.empty())
    Logger::log_srv(_server_name, "going out of scope");
  if (!_clients.empty()) {
    Logger::log_srv(_server_name, "removing all clients");
    // _clients.clear();
    _removeAllClients();
  }
}

// settings up sockets per server.
//
// the canonical workflow here is
//
//	 1) socket
//	 2) bind
//	 3) listen
//
// ... and accept, for the server, or connect for the client.
//
// socket() explained:
//
//	 - AF_INET:       socket for communication via IPv4
//	 - SOCK_STREAM:   we want a reliable, bidirectional, byte-stream communi-
//										cation channel
//	 - SOCK_NONBLOCK: save a call to fcntl
//	 - 0:             use default protocol
//
//	 NOTE: maybe we also want SOCK_CLOEXEC here as we maybe do not want our
//	 sockets to be open in child processes.
//
// setsockopt() explained:
//
// 	- SOL_SOCKET: this is the level the sockopt will be applied to. SOL_SOCKET
// 		is the sockets API level, another level would be SOL_IP.
// 	- SO_REUSEADDR: avoid EADDRINUSE if webserv (or one of the servers) is being
// 		restarted.
// 	- SO_REUSEPORT (not used): only useful for multi-threaded servers. allows
// 		binding socket to the same src_addr:port pair
//
// listen() explained:
//
//	- SOMAXCONN: 4096 on my system, maximum number of connections in the backlog
//		of listen
void VServer::_setupSockets()
{
  std::map< str, std::set<u16> >::const_iterator it =
      _cfg.getInterfaces().begin();
  while (it != _cfg.getInterfaces().end()) {
    str addr = it->first;

    // NOTE: when we get here there must be a port for every interface. so in
    // some earlier parsing step we will have to add port 80 for interfaces
    // without specified ports.
    // No! We demand that interfaces always come in the form addr:port!
    std::set<u16> ports = it->second;

    // better safe then sorry
    if (ports.size() == 0) {
      it++;
      continue;
    }

    std::pair<str, int> addrFd;
    for (std::set<u16>::iterator itp = ports.begin(); itp != ports.end(); itp++)
    {
      addrFd = Socket::bindSocket(addr, *itp);
      if (addrFd.second == -1)
        continue;
      if (listen(addrFd.second, SOMAXCONN) == -1) {
        close(addrFd.second);
        continue;
      }
      _listen_fds.insert(addrFd.second);
      _ports.insert(*itp);

      // FIXME: maybe later refactor to only use one of these

      // store addr as given in cfg and port
      _activeInterfaces[addr].insert(*itp);

      // store IP and port
      _activeAddrPortPairs[addrFd.first].insert(*itp);
    }
    it++;
  }
  if (_listen_fds.size() == 0)
    throw(ServerInitException("could not setup a single socket"));
}

// Init a server. If initialization fails after call to socket we would be left
// with a open fd, so we need to close it for proper cleanup
void VServer::init()
{
  try {
    _setupSockets();
  } catch (const VServer::ServerInitException& e) {
    throw;
  }
  Logger::log_srv(_server_name, "initialized!");
}

// TODO: maybe design some more helper functions to make this more compact.
//
// The crazy syntax of this block
//
// 	std::pair<std::map<int, Client>::iterator, bool> insertReturn =
// 		_clients.insert(
// 				std::pair<int, Client>(client_fd, Client(client_fd, clock())));
//
// explained: std::map::insert returns a std::pair where the `first` member is
// an iterator pointing to the newly inserted element (a pair again) and the
// `second` member is a bool flag indicating wether a new element could be
// inserted or not. if the key already exists the new element can not be
// inserted.
Client *VServer::addClient(int fd)
{
  if (_listen_fds.find(fd) == _listen_fds.end())
    throw(ServerException("server_fds = " + getSetAsStr(_listen_fds) +
        ", conn_fd = " + int2str(fd)));

  Logger::log_srv(_server_name, "accepting new conn on fd " + int2str(fd));

  struct sockaddr_in client_addr;
  socklen_t          client_addr_len = sizeof(client_addr);

  int client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_addr_len);
  if (client_fd == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      Logger::log_err("accept failed with EAGAIN || WOULDBLOCK");
    else
      Logger::log_err("accept failed: " + getErrStr());
  }
  Logger::log_srv(
      _server_name, "Client connected from " + getAddrPortStr4(client_addr));

  if (setFdNonBlocking(client_fd) == -1)
    throw(ServerException("could not set new clients fd non-blocking"));

  std::string hostname(inAddrToStr(client_addr.sin_addr));
  Client     *newCli = new Client(client_fd, hostname, client_addr.sin_port);

  std::pair<std::map<int, Client *>::iterator, bool> insertReturn =
      _clients.insert(std::pair<int, Client *>(client_fd, newCli));
  if (insertReturn.second == false)
    throw(ServerException(
        "could not insert new client into Server's _clients map"));

  return (insertReturn.first->second);
}

// remove all traces of a client from the Server _and_ close the socket. things
// to be cleaned up:
//
// 	1) remove from clients list
// 	2) remove from pair from clientFdMap
// 	3) close socket
void VServer::removeClient(int fd)
{
  if (_clients.find(fd) == _clients.end())
    throw(
        ServerException("fd " + int2str(fd) + " in removeClient() not found"));
  delete _clients.find(fd)->second;
  _clients.erase(fd);
}

void VServer::_removeAllClients()
{
  for (std::map<int, Client *>::iterator it = _clients.begin();
      it != _clients.end();
      it++)
    delete it->second;
  _clients.clear();
}

// INFO: this is another heart-piece of this webserv.
int VServer::handleEvent(const struct epoll_event& ev, int client_fd)
{
  int return_value = 0;
  if (ev.events & EPOLLIN)
    return_value = _reqHandler.readRequest(client_fd);
  if (ev.events & EPOLLOUT)
    return_value = _reqHandler.writeResponse(client_fd);
  return (return_value);
}

// close all socket fds
// not handling any errors for close except displaying a short warning msg.
//
// TODO: in principle i could try and have small retry loop here
void VServer::cleanup()
{
  for (std::set<int>::iterator it = _listen_fds.begin();
      it != _listen_fds.end();
      it++)
    if (*it != -1 && close(*it) == -1)
      Logger::log_warn("VServer::cleanup: failed to close fd " + int2str(*it) +
          " with error: " + getErrStr());
}

////////////////////////////////////////////////////////////////////////////////
/// exceptions

VServer::ServerInitException::ServerInitException(const std::string& msg):
  std::runtime_error("ServerInitException: " + msg)
{}

VServer::ServerException::ServerException(const std::string& msg):
  std::runtime_error("ServerException: " + msg)
{}
