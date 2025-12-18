/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_Utils.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 12:11:11 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/18 23:20:55 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Server.hpp"
#include "utils.hpp"

#include <iostream>

////////////////////////////////////////////////////////////////////////////////
/// Server methods of minor importance

// the getters
uint16_t		Server::getPort() const { return (_port); }
in_addr_t		Server::getHost() const { return (_host); }
std::string Server::getServerName() const { return (_server_name); }
std::string Server::getRoot() const { return (_root); }
sockaddr_in Server::getServerAddr() const { return (_server_addr); }
int					Server::getListenFd() const { return (_listen_fd); }

// the setters
void Server::setPort(uint16_t port) { _port = port; }
void Server::setHost(in_addr_t host) { _host = host; }
void Server::setServerName(std::string name) { _server_name = name; }
void Server::setRoot(std::string root) { _root = root; }
void Server::setServerAddr(sockaddr_in server_addr)
{
	this->_server_addr = server_addr;
}
void Server::setListenFd(int listen_fd) { _listen_fd = listen_fd; }

void Server::printCfg() const
{
	struct in_addr host_addr;
	host_addr.s_addr = htonl(_host);
	Logger::log_msg("  server_name: \"" + _server_name + "\"");
	Logger::log_msg("  port: " + int2str(_port));
	Logger::log_msg("  root: " + _root);
	Logger::log_msg("  listen_fd: " + int2str(_listen_fd));
	Logger::log_msg("  host: " + inaddrToStr(host_addr));
}

void Server::printClients()
{
	Logger::log_srv(_server_name, "Printing Clients:");
	if (_clients.empty()) {
		Logger::log_msg("-> Server has got no clients");
		return;
	}
	for (std::map<int, Client *>::iterator it = _clients.begin();
			it != _clients.end(); it++)
	{
		std::cout << "  fd: " << it->second->getFd() << std::endl;
	}
}

// returns true if fd is a member of _clients belonging to this server.
bool Server::isValidClientFd(int fd)
{
	return (_clients.find(fd) != _clients.end());
}
