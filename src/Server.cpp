/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:51:23 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/12 15:10:41 by fmaurer          ###   ########.fr       */
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
// NOTE: skip closing socket for now
Server::~Server()
{
	// Logger::log_msg("closing socket for server \"" + _server_name + "\"");
	// if (close(_listen_fd) == -1)
	// 	Logger::log_err("could not close server socket");
}

std::string Server::getServerName() const { return (_server_name); }

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

// NEXT:
// FIXME: refac! make a class for it
// FIXME: move to Webserv class
void Server::_setupEpoll()
{
	_epoll_fd = epoll_create1(0);
	if (_epoll_fd == -1)
		throw(ServerInitException("epoll_create1 failed"));

	struct epoll_event ev;
	ev.events	 = EPOLLIN;
	ev.data.fd = _listen_fd;

	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _listen_fd, &ev) == -1) {
		close(_epoll_fd);
		throw(ServerInitException("epoll_ctl failed"));
	}
}

void Server::init()
{
	try {
		_setupSocket();
		_setupEpoll();
	} catch (const Server::ServerInitException& e) {
		std::cout << "e.what(): " << e.what() << std::endl;
	}
	Logger::log_msg("server \"" + _server_name + "\" initialized!");
}

Server::ServerInitException::ServerInitException(const std::string& msg):
	std::runtime_error("ServerInitException: " + msg)
{}

Server::ServerRunException::ServerRunException(const std::string& msg):
	std::runtime_error("ServerRunException: " + msg)
{}

void Server::run()
{

	const int					 MAX_EVENTS = 10;
	struct epoll_event events[MAX_EVENTS];

	if (_epoll_fd == -1)
		throw(
				ServerInitException("Server needs to be initialized in order to run"));

	while (true) {
		// int nfds = epoll_wait(_epoll_fd, events, MAX_EVENTS, -1);
		Logger::log_msg("server \"" + _server_name + "\" epoll_waiting");
		int nfds = epoll_wait(_epoll_fd, events, MAX_EVENTS, 100);
		if (nfds == -1)
			throw(ServerRunException("epoll wait failed"));

		for (int i = 0; i < nfds; ++i) {
			if (events[i].data.fd == _listen_fd) {
				// Accept new connection
				struct sockaddr_in client_addr;
				socklen_t					 client_addr_len = sizeof(client_addr);
				int								 client_fd			 = accept(_listen_fd,
						 (struct sockaddr *)&client_addr,
						 &client_addr_len);
				if (client_fd == -1) {
					Logger::log_err("accept failed");
					continue;
				}

				// Add client socket to epoll
				struct epoll_event client_ev;
				client_ev.events	= EPOLLIN;
				client_ev.data.fd = client_fd;
				if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev) == -1) {
					Logger::log_err("epoll_ctl failed");
					close(client_fd);
					continue;
				}
			}
			else {
				// Handle client data
				char		buffer[1024];
				ssize_t bytes_read =
						read(events[i].data.fd, buffer, sizeof(buffer) - 1);
				if (bytes_read <= 0) {
					if (bytes_read == 0) {
						std::cout << "Client disconnected\n";
					}
					else {
						Logger::log_err("read failed");
					}
					close(events[i].data.fd);
				}
				else {
					buffer[bytes_read] = '\0';
					std::cout << "Received: " << buffer << std::endl;
				}
			}
		}
	}
	close(_epoll_fd);
}
