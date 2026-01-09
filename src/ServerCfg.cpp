/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerCfg.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 08:35:42 by fmaurer           #+#    #+#             */
/*   Updated: 2026/01/09 17:00:08 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerCfg.hpp"
#include "utils.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>

//// OCF

// Default Constructor with default values.
// NOTE: do all non-generic setting in ServerSetup
ServerCfg::ServerCfg()
{
	_port				 = 0;
	_host				 = 0;
	_server_name = "";
	_root				 = "";
	memset(&_server_addr, 0, sizeof(_server_addr));
}

ServerCfg::ServerCfg(const ServerCfg& other)
{
	_port				 = other._port;
	_host				 = other._host;
	_server_name = other._server_name;
	_root				 = other._root;
	_server_addr = other._server_addr;
}

ServerCfg& ServerCfg::operator=(const ServerCfg& other)
{
	if (this != &other) {
		_port				 = other._port;
		_host				 = other._host;
		_server_name = other._server_name;
		_root				 = other._root;
		_server_addr = other._server_addr;
	}
	return (*this);
}

ServerCfg::~ServerCfg() {}

//// OCF end

void ServerCfg::setPort(uint16_t port) { _port = port; }
void ServerCfg::setHost(in_addr_t host) { _host = host; }
void ServerCfg::setServerName(std::string name) { _server_name = name; }
void ServerCfg::setRoot(std::string root) { _root = root; }
void ServerCfg::setServerAddr(sockaddr_in server_addr)
{
	this->_server_addr = server_addr;
}

// void ServerCfg::setClientMaxBodySize(unsigned long max_body_size)
// {
// 	this->_client_max_body_size = max_body_size;
// }
// void ServerCfg::setIndex(std::string index) { this->_index = index; }
// void ServerCfg::setLocations(std::vector<Location> locations)
// {
// 	this->_locations = locations;
// }

uint16_t		ServerCfg::getPort() const { return (_port); }
in_addr_t		ServerCfg::getHost() const { return (_host); }
std::string ServerCfg::getServerName() const { return (_server_name); }
std::string ServerCfg::getRoot() const { return (_root); }
sockaddr_in ServerCfg::getServerAddr() const { return (_server_addr); }

// unsigned long
// ServerCfg::getClientMaxBodySize() {} std::string
// ServerCfg::getIndex() {} std::vector<Location> ServerCfg::getLocations() {}

// uint16_t	_port;
// in_addr_t _host;
// std::string				 _server_name;
// std::string				 _root;
// struct sockaddr_in _server_addr;
//
// with our own conversion functions for ip-addrs.
void ServerCfg::printCfg() const
{
	struct in_addr host_addr;
	host_addr.s_addr = htonl(_host);
	std::cout << "  server_name: \"" << _server_name << "\"" << std::endl;
	std::cout << "  port: " << _port << std::endl;
	std::cout << "  root: " << _root << std::endl;

	// FIXME: remove inet_ntoa because we cannot use it but keep it for now to see
	// if our function is fine.
	// std::cout << "  host: " << inet_ntoa(host_addr) << std::endl;
	std::cout << "  host: " << inaddrToStr(host_addr) << std::endl;
}
