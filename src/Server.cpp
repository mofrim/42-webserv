/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:51:23 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/03 15:42:56 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Server.hpp"
#include "utils.hpp"

#include <iostream>
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
}

// TODO: implement
Server::Server(const Server& other) { (void)other; }

// TODO: implement
Server& Server::operator=(const Server& other)
{
	(void)other;
	return (*this);
}

// TODO: implement! there will definitely be stuff to do here, like freeing the
// socket, shutting down connections etc.
Server::~Server()
{
	Logger::log_msg("closing socket for server \"" + _server_name + "\"");
	if (close(_listen_fd) == -1)
		Logger::log_err("could not close server socket");
}

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
			"server \"" + _server_name + "\" listening on fd " + lit2str(_listen_fd));
}

void Server::init()
{
	try {
		_setupSocket();

	} catch (const Server::ServerInitException& e) {
		std::cout << "e.what(): " << e.what() << std::endl;
	}
}

Server::ServerInitException::ServerInitException(const std::string& msg):
	std::runtime_error("ServerInitException" + msg)
{}
