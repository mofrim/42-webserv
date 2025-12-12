/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:51:23 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/12 18:05:43 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Server.hpp"
#include "utils.hpp"

#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>

// TODO: implement
Server::Server() {}

Server::Server(const ServerCfg& srvcfg)
{
	_port				 = srvcfg.getPort();
	_host				 = srvcfg.getHost();
	_server_name = srvcfg.getServerName();
	_root				 = srvcfg.getRoot();
	_listen_fd	 = srvcfg.getListenFd();
	_server_addr = srvcfg.getServerAddr();

	// FIXME: move to Webserv class!!!
	_epoll_fd = -1;
}

// TODO: implement
Server::Server(const Server& other)
{
	if (this != &other) {
		_port				 = other._port;
		_host				 = other._host;
		_server_name = other._server_name;
		_root				 = other._root;
		_listen_fd	 = other._listen_fd;
		_server_addr = other._server_addr;

		// FIXME: move to Webserv class!!!
		_epoll_fd = other._epoll_fd;
	}
}

// TODO: implement
Server& Server::operator=(const Server& other)
{
	(void)other;
	return (*this);
}

// TODO: implement! there will definitely be stuff to do here, like freeing the
// socket, shutting down connections etc.
// FIXME: close sockets in Webserv class
Server::~Server()
{
	// Logger::log_msg("closing socket for server \"" + _server_name + "\"");
	// if (close(_listen_fd) == -1)
	// 	Logger::log_err("could not close server socket");
}

// FIXME: make the socket non-blocking!
void Server::_setupSocket()
{
	_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listen_fd == -1)
		throw(ServerInitException("Socket setup failed"));

	// set socket options (e.g., reuse address)
	// FIXME: clarify and document params here
	int opt = 1;
	if (setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		throw(ServerInitException("setsockopt failed"));

	// bind the socket
	// TODO: document
	if (bind(_listen_fd,
					(struct sockaddr *)&_server_addr,
					sizeof(_server_addr)) == -1)
		throw(ServerInitException("bind failed"));

	// Listen for connections
	// TODO: document
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

Server::ServerInitException::ServerInitException(const std::string& msg):
	std::runtime_error("ServerInitException: " + msg)
{}

// the getters
uint16_t		Server::getPort() const { return (_port); }
in_addr_t		Server::getHost() const { return (_host); }
std::string Server::getServerName() const { return (_server_name); }
std::string Server::getRoot() const { return (_root); }
sockaddr_in Server::getServerAddr() const { return (_server_addr); }
int					Server::getListenFd() const { return (_listen_fd); }
