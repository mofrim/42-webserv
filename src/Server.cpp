/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:51:23 by fmaurer           #+#    #+#             */
/*   Updated: 2026/01/12 17:40:44 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Server.hpp"
#include "utils.hpp"

#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

Server::Server(): _reqHandler(this)
{
	_port				 = 0;
	_host				 = 0;
	_server_name = "";
	_root				 = "";
	_listen_fd	 = -1;
	_setupFailed = false;
	memset(&_server_addr, 0, sizeof(_server_addr));
}

Server::Server(const ServerCfg& srvcfg): _reqHandler(this)
{
	_listen_fd	 = -1;
	_port				 = srvcfg.getPort();
	_host				 = srvcfg.getHost();
	_server_name = srvcfg.getServerName();
	_root				 = srvcfg.getRoot();
	_server_addr = srvcfg.getServerAddr();
	_cfg				 = srvcfg;
	_setupFailed = false;
}

Server::Server(const Server& other): _reqHandler(this)
{
	if (this != &other) {
		_port				 = other._port;
		_host				 = other._host;
		_server_name = other._server_name;
		_root				 = other._root;
		_listen_fd	 = other._listen_fd;

		_server_addr = other._server_addr;
		_setupFailed = other._setupFailed;
		_cfg				 = other._cfg;
	}
}

// NOTE: only for this use-case implemented the copy assignment constructor for
// RequestHandler. only place this is used: Webserv::_setupServers.
Server& Server::operator=(const Server& other)
{
	if (this != &other) {
		_port				 = other._port;
		_host				 = other._host;
		_server_name = other._server_name;
		_root				 = other._root;
		_listen_fd	 = other._listen_fd;

		_server_addr = other._server_addr;
		_setupFailed = other._setupFailed;
		_cfg				 = other._cfg;
		_clients		 = other._clients;
		_reqHandler	 = RequestHandler(this);
	}
	return (*this);
}

// a little hack to avoid printing the "out of scope msg" for tmp servers not
// fully initialized
Server::~Server()
{
	if (_listen_fd != -1)
		Logger::log_srv(_server_name, "going out of scope");
	if (!_clients.empty()) {
		Logger::log_srv(_server_name, "removing all clients");
		_clients.clear();
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
// TODO: use Socket-class here.
void Server::_setupSocket()
{
	_listen_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (_listen_fd == -1)
		throw(ServerInitException("socket failed"));

	int opt = 1;
	if (setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		throw(ServerInitException("setsockopt failed"));

	if (bind(_listen_fd, (struct sockaddr *)&_server_addr, sizeof(_server_addr))
			== -1)
		throw(ServerInitException("bind failed"));

	if (listen(_listen_fd, SOMAXCONN) == -1)
		throw(ServerInitException("listen failed"));

	Logger::log_srv(_server_name, "listening on fd " + int2str(_listen_fd));
}

// Init a server. If initialization fails after call to socket we would be left
// with a open fd, so we need to close it for proper cleanup
void Server::init()
{
	try {
		_setupSocket();
	} catch (const Server::ServerInitException& e) {
		if (_listen_fd != -1)
			close(_listen_fd);
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
Client *Server::addClient(int fd)
{
	if (fd != _listen_fd)
		throw(ServerException(
				"server_fd = " + int2str(_listen_fd) + ", conn_fd = " + int2str(fd)));
	Logger::log_srv(_server_name, "accepting new conn on fd " + int2str(fd));

	struct sockaddr_in client_addr;
	socklen_t					 client_addr_len = sizeof(client_addr);

	int client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_addr_len);
	if (client_fd == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			Logger::log_err("accept failed with EAGAIN || WOULDBLOCK");
		else
			Logger::log_err(std::string("accept failed: ", *strerror(errno)));
	}
	std::string hostname(inaddrToStr(client_addr.sin_addr));
	Logger::log_srv(_server_name,
			"Client connected from " + hostname + ":"
					+ int2str(client_addr.sin_port));

	if (setFdNonBlocking(client_fd) == -1)
		throw(ServerException("could not set new clients fd non-blocking"));

	Client *newCli = new Client(client_fd, hostname, client_addr.sin_port);
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
void Server::removeClient(int fd)
{
	if (_clients.find(fd) == _clients.end())
		throw(
				ServerException("fd " + int2str(fd) + " in removeClient() not found"));
	delete _clients.find(fd)->second;
	_clients.erase(fd);
}

void Server::removeAllClients()
{
	for (std::map<int, Client *>::iterator it = _clients.begin();
			it != _clients.end(); it++)
		delete it->second;
	_clients.clear();
}

// INFO: this is another heart-piece of this webserv.
int Server::handleEvent(const struct epoll_event& ev, int client_fd)
{
	int return_value = 0;
	if (ev.events & EPOLLIN)
		return_value = _reqHandler.readRequest(client_fd);
	if (ev.events & EPOLLOUT)
		return_value = _reqHandler.writeResponse(client_fd);
	return (return_value);
}

////////////////////////////////////////////////////////////////////////////////
/// exceptions

Server::ServerInitException::ServerInitException(const std::string& msg):
	std::runtime_error("ServerInitException: " + msg)
{}

Server::ServerException::ServerException(const std::string& msg):
	std::runtime_error("ServerException: " + msg)
{}
