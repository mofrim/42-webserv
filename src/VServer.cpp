/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServer.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:51:23 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/17 10:13:37 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Socket.hpp"
#include "VServer.hpp"
#include "utils.hpp"

#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

// --------------------------------=[ OCF ]=-------------------------------- //

VServer::VServer()
{
  _srvName     = "";
  _setupFailed = false;
  _root        = "./www";
  _maxBodySize = MAX_BODY_SIZE;
}

// FIXME: think about if assignment and stuff like this really make sense for my
// vserver classes
VServer::VServer(const VServer& o)
{
  if (this != &o) {
    _srvName          = o._srvName;
    _listenFds        = o._listenFds;
    _virtualFds       = o._virtualFds;
    _setupFailed      = o._setupFailed;
    _cfgInterfaces    = o._cfgInterfaces;
    _activeInterfaces = o._activeInterfaces;
    _fdIfaceMap       = o._fdIfaceMap;
    _clients          = o._clients;
    _routes           = o._routes;
    _root             = o._root;
    _errPages         = o._errPages;
    _maxBodySize      = o._maxBodySize;
  }
}

// NOTE: only for this use-case implemented the copy assignment constructor for
// RequestHandler. only place this is used: Webserv::_setupServers.
VServer& VServer::operator=(const VServer& o)
{
  if (this != &o) {
    _srvName          = o._srvName;
    _listenFds        = o._listenFds;
    _virtualFds       = o._virtualFds;
    _setupFailed      = o._setupFailed;
    _cfgInterfaces    = o._cfgInterfaces;
    _activeInterfaces = o._activeInterfaces;
    _fdIfaceMap       = o._fdIfaceMap;
    _routes           = o._routes;
    _clients          = o._clients;
    _root             = o._root;
    _errPages         = o._errPages;
    _maxBodySize      = o._maxBodySize;
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
  if (!_listenFds.empty())
    Logger::logSrv(_srvName, "going out of scope");
  if (!_clients.empty()) {
    Logger::logSrv(_srvName, "removing all clients");
    // _clients.clear();
    _removeAllClients();
  }
  cleanup();
}

// -----------------------------=[ Extra CTOR ]=----------------------------- //

VServer::VServer(const VServerCfg& cfg)
{
  _srvName       = cfg.getServerName();
  _cfgInterfaces = cfg.getInterfaces();
  _routes        = cfg.getRoutes();
  _maxBodySize   = cfg.getMaxBodySize();
  _errPages      = cfg.getErrPages();
  _root          = cfg.getRoot();
  _setupFailed   = false;
}

// ---------------------------=[ Other routines ]=--------------------------- //

// Init a server. If initialization fails after call to socket we would be left
// with a open fd, so we need to close it for proper cleanup
void VServer::init(std::vector<VServer>::iterator begin,
    std::vector<VServer>::iterator                cur,
    char                                        **envp)
{
  try {
    _setupSockets(begin, cur);
  } catch (const VServer::ServerInitException& e) {
    throw;
  }
  Logger::logSrv(_srvName, "initialized!");
  _envp = envp;
}

// settings up sockets per server.
void VServer::_setupSockets(
    std::vector<VServer>::iterator begin, std::vector<VServer>::iterator cur)
{

  std::map< str, std::set<u16> >::iterator it = _cfgInterfaces.begin();
  while (it != _cfgInterfaces.end()) {

    // resolving possible host-/domain-name to ip addr here for better
    // comparability. if we cannot resolve ourselves -> skip iface
    //
    // ATTENTION if virtual server matching is malfunctioning this might be the
    // first place to look into
    str hostname = it->first;
    str ipAddr   = Socket::resolveAddr(hostname);
    if (ipAddr.empty()) {
      it = eraseIt(_cfgInterfaces, it);
      continue;
    }

    // NOTE: when we get here there must be a port for every interface. so in
    // some earlier parsing step we will have to add port 80 for interfaces
    // without specified ports.
    // No! We demand that interfaces always come in the form addr:port!

    std::set<u16> ports = it->second;

    // better safe then sorry
    if (ports.size() == 0) {
      it = eraseIt(_cfgInterfaces, it);
      continue;
    }

    t_AddrinfoReturn ipCnameFd = {.ip = "", .cname = "", .fd = -1};

    std::set<u16>::iterator itp = ports.begin();

    // THIS MUST BE A WHILE LOOP because of eraseIt + continue being used in it
    while (itp != ports.end()) {

      int sockFd = -1;
      if ((sockFd = _findVirtualBuddy(begin, cur, ipAddr, *itp)) >= 0) {
        Logger::logMsg("Found real virtual server: " + _srvName + " - " +
            ipAddr + ":" + int2str(*itp));
        _virtualFds.insert(sockFd);
      }
      // the _srvName did match
      else if (sockFd == -42) {
        Logger::logWarn(
            "Same interface AND srvName! Dropping duplicate iface for " +
            ipAddr + ":" + int2str(*itp));
        itp = eraseIt(ports, itp);
        continue;
      }
      else {
        ipCnameFd = Socket::bindSocket(hostname, *itp);
        sockFd    = ipCnameFd.fd;

        if (sockFd == -1) {
          itp = eraseIt(ports, itp);
          continue;
        }

        if (listen(sockFd, SOMAXCONN) == -1) {
          close(sockFd);
          itp = eraseIt(ports, itp);
          continue;
        }
      }

      _listenFds.insert(sockFd);
      _ports.insert(*itp);

      // FIXME: maybe later refactor to only use one of these

      // store IP and port
      // _activeInterfaces[ipCnameFd.ip].insert(*itp);
      _addActiveIface(ipCnameFd, *itp);

      // store fd and iface:port mapping for use in virtual server
      // identification
      _fdIfaceMap[sockFd] = std::make_pair(ipAddr, *itp);

      itp++;
    }

    if (ports.size() == 0) {
      it = eraseIt(_cfgInterfaces, it);
      continue;
    }
    it++;
  }
  if (_listenFds.size() == 0)
    throw(ServerInitException("Could not init any interface!"));
}

int VServer::_findVirtualBuddy(std::vector<VServer>::iterator begin,
    std::vector<VServer>::iterator                            cur,
    const str&                                                addr,
    u16                                                       port)
{
  int vfd = -1;

  for (std::vector<VServer>::iterator it = begin; it != cur; ++it) {
    if ((vfd = it->_isActiveIface(addr, port)) != -1) {
      if (it->_srvName == _srvName)
        return -42;
      return vfd;
    }
  }
  return vfd;
}

// lookup the given addr:port in the _fdIfaceMap of a VServer. if it is found
// return the corresponding fd.
int VServer::_isActiveIface(const str& addr, u16 port) const
{
  std::map< int, std::pair<str, u16> >::const_iterator it = _fdIfaceMap.begin();
  while (it != _fdIfaceMap.end()) {
    if (it->second.first == addr && it->second.second == port)
      return it->first;
    ++it;
  }
  return -1;
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
Client *VServer::addClient(Webserv *wsrv, int fd)
{
  if (_listenFds.find(fd) == _listenFds.end())
    throw(ServerException("server_fds = " + getSetAsStr(_listenFds) +
        ", conn_fd = " + int2str(fd)));

  struct sockaddr_in client_addr;
  socklen_t          client_addr_len = sizeof(client_addr);

  int client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_addr_len);
  if (client_fd == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      Logger::logErr("accept failed with EAGAIN || WOULDBLOCK");
    else
      Logger::logErr("accept failed: " + getErrStr());
  }
  Logger::logSrv(_srvName,
      "Client " + getAddrPortStr4(client_addr) + " connected on fd " +
          int2str(client_fd));

  if (setFdNonBlocking(client_fd) == -1)
    throw(ServerException("could not set new clients fd non-blocking"));

  std::string hostname(inAddrToStr(client_addr.sin_addr));
  Client     *newCli =
      new Client(wsrv, client_fd, this, hostname, client_addr.sin_port);

  if (newCli == NULL)
    throw std::runtime_error(
        "(VServer::addClient) Could not allocate new Client!");

  // set server port in new client
  newCli->setVsrvPort(_fdIfaceMap[fd].second);

  _clients[client_fd] = newCli;
  return (newCli);
}

void VServer::addClient(Client *cli) { _clients[cli->getFd()] = cli; }

// remove all traces of a client from the Server _and_ close the socket. things
// to be cleaned up:
//
// 	1) remove from clients list
// 	2) remove from pair from clientFdMap
// 	3) close socket
void VServer::deleteClient(int fd)
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

// close all socket fds
// not handling any errors for close except displaying a short warning msg.
//
// TODO: in principle i could try and have small retry loop here
void VServer::cleanup()
{
  for (std::set<int>::iterator it = _listenFds.begin(); it != _listenFds.end();
      it++)
  {
    if (*it != -1 && !isVirtualFd(*it))
      if (close(*it) == -1)
        Logger::logWarn("VServer::cleanup: failed to close fd " + int2str(*it) +
            " with error: " + getErrStr());
  }
}

// zap through routes and find the longest matching path
const Route& VServer::matchRoute(constr& target)
{
  for (std::map<str, Route>::const_iterator it = _routes.begin();
      it != _routes.end();
      ++it)
  {
    constr& path = it->second.getPath();
    if (target == path)
      return it->second;
  }

  return _routes["/"];
}

char **VServer::getEnvp() { return _envp; }
