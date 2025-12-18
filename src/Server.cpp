/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:51:23 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/18 17:28:23 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Server.hpp"
#include "utils.hpp"

#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

Server::Server()
{
	_port				 = 0;
	_host				 = 0;
	_server_name = "";
	_root				 = "";
	// FIXME: is this a sensible initial value?
	_listen_fd = -1;
	memset(&_server_addr, 0, sizeof(_server_addr));
}

Server::Server(const ServerCfg& srvcfg)
{
	_port				 = srvcfg.getPort();
	_host				 = srvcfg.getHost();
	_server_name = srvcfg.getServerName();
	_root				 = srvcfg.getRoot();
	_listen_fd	 = srvcfg.getListenFd();
	_server_addr = srvcfg.getServerAddr();
}

Server::Server(const Server& other)
{
	if (this != &other) {
		_port				 = other._port;
		_host				 = other._host;
		_server_name = other._server_name;
		_root				 = other._root;
		_listen_fd	 = other._listen_fd;
		_server_addr = other._server_addr;
	}
}

// TODO: implement
Server& Server::operator=(const Server& other)
{
	(void)other;
	return (*this);
}

Server::~Server()
{
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
//
// listen() explained:
//
//	- SOMAXCONN: 4096 on my system, maximum number of connections in the backlog
//		of listen
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

	Logger::log_msg(
			"server \"" + _server_name + "\" listening on fd " + int2str(_listen_fd));
}

void Server::init()
{
	try {
		_setupSocket();
	} catch (const Server::ServerInitException& e) {
		std::cout << "e.what(): " << e.what() << std::endl;
	}
	Logger::log_msg("server \"" + _server_name + "\" initialized!");
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
	Logger::log_dbg0("accepting new conn on fd " + int2str(fd));

	struct sockaddr_in client_addr;
	socklen_t					 client_addr_len = sizeof(client_addr);

	int client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_addr_len);
	if (client_fd == -1) {
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			Logger::log_err("accept failed with EAGAIN || WOULDBLOCK");
		else
			Logger::log_err(std::string("accept failed: ", *strerror(errno)));
	}
	Logger::log_dbg0(
			"Client connected from address " + inaddrToStr(client_addr.sin_addr));

	if (setFdNonBlocking(client_fd) == -1)
		throw(ServerException("could not set new clients fd non-blocking"));

	Client *newCli = new Client(client_fd, clock());
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

// NEXT: this is a big one.... _ALL_ request handling logic continues here
// FIXME: ASAP implement clean client removal!
int Server::handleEvent(const struct epoll_event& ev, int client_fd)
{
	(void)ev;

	if (_clients.find(client_fd) == _clients.end())
		throw(ServerException("client_fd in handleEvent() not found"));

	Logger::log_srv(_server_name, "reading from socket" + int2str(client_fd));
	char		buffer[1024];
	ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

	// if there was a read error, or there was nothing to read, return 1 in order
	// to signal deletion from epoll interest list
	if (bytes_read <= 0) {
		if (bytes_read == 0) {
			Logger::log_warn("Client disconnected");
			Logger::log_warn("closing client conn on fd " + int2str(client_fd));
		}
		else
			Logger::log_err(
					"read failed, errno: " + int2str(errno) + " = " + strerror(errno));
		return (-1);
	}
	// NEXT:
	// IDEA: maybe it is enough to pass the server cfg of the server the
	// client is connected to here?!?!
	// Biiiiiiig QUESTION: how and where is it elegant to really handle the
	// request!?!?!?!? Is there some max-size for a request??!?!?!?!
	// what if i start handling the request down here but do not have any idead
	// about the server config!?!? this would be complete bullshit!!!!
	buffer[bytes_read] = '\0';
	std::cout << "Received:\n" << buffer << std::endl;
	return (0);
}

////////////////////////////////////////////////////////////////////////////////
/// exceptions

Server::ServerInitException::ServerInitException(const std::string& msg):
	std::runtime_error("ServerInitException: " + msg)
{}

Server::ServerException::ServerException(const std::string& msg):
	std::runtime_error("ServerException: " + msg)
{}
