/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server_Utils.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 12:11:11 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/17 16:40:07 by fmaurer          ###   ########.fr       */
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
	Logger::log_dbg0("  server_name: \"" + _server_name + "\"");
	Logger::log_dbg0("  port: " + int2str(_port));
	Logger::log_dbg0("  root: " + _root);
	Logger::log_dbg0("  listen_fd: " + int2str(_listen_fd));

	// FIXME: remove inet_ntoa because we cannot use it but keep it for now to
	// see if our function is fine. std::cout << "  host: " <<
	// inet_ntoa(host_addr) << std::endl;
	Logger::log_dbg0("  host: " + inaddrToStr(host_addr));
}

void Server::printClients()
{
	Logger::log_dbg0("Printing Clients of Server " + _server_name + ":");
	if (_clients.empty()) {
		Logger::log_dbg0("-> Server has got no clients");
		return;
	}
	for (std::list<Client>::iterator it = _clients.begin(); it != _clients.end();
			it++)
	{
		std::cout << "  fd: " << it->getFd() << std::endl;
	}
}
