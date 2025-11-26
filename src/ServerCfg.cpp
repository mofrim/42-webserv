/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerCfg.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 08:35:42 by fmaurer           #+#    #+#             */
/*   Updated: 2025/11/26 10:14:20 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerCfg.hpp"

#include <cstring>
#include <iostream>

//// OCF

// Default Constructor with default values.
// NOTE: do all non-generic setting in ServerSetup
ServerCfg::ServerCfg()
{
	this->_port				 = 0;
	this->_host				 = 0;
	this->_server_name = "";
	this->_root				 = "";
	this->_listen_fd	 = 0;
	memset(&this->_server_addr, 0, sizeof(this->_server_addr));

	// this->_port				 = 4284;
	// this->_host				 = 0;
	// this->_server_name = "webserv";
	// this->_root				 = "www";
	// this->_listen_fd	 = 0;
	//
	// // initialize default server addr.
	// memset(&this->_server_addr, 0, sizeof(this->_server_addr));
	// this->_server_addr.sin_family			 = AF_INET;
	// this->_server_addr.sin_addr.s_addr = INADDR_ANY;
	// this->_server_addr.sin_port				 = htons(this->_port);
}

ServerCfg::ServerCfg(const ServerCfg& other)
{
	this->_port				 = other._port;
	this->_host				 = other._host;
	this->_server_name = other._server_name;
	this->_root				 = other._root;
	this->_listen_fd	 = other._listen_fd;
	this->_server_addr = other._server_addr;
}

ServerCfg& ServerCfg::operator=(const ServerCfg& other)
{
	if (this != &other) {
		this->_port				 = other._port;
		this->_host				 = other._host;
		this->_server_name = other._server_name;
		this->_root				 = other._root;
		this->_listen_fd	 = other._listen_fd;
		this->_server_addr = other._server_addr;
	}
	return (*this);
}

ServerCfg::~ServerCfg() {}

//// OCF end

void ServerCfg::setPort(uint16_t port) { this->_port = port; }
void ServerCfg::setHost(in_addr_t host) { this->_host = host; }
void ServerCfg::setServerName(std::string name) { this->_server_name = name; }
void ServerCfg::setRoot(std::string root) { this->_root = root; }
void ServerCfg::setServerAddr(sockaddr_in server_addr)
{
	this->_server_addr = server_addr;
}
void ServerCfg::setListenFd(int listen_fd) { this->_listen_fd = listen_fd; }

// void ServerCfg::setClientMaxBodySize(unsigned long max_body_size)
// {
// 	this->_client_max_body_size = max_body_size;
// }
// void ServerCfg::setIndex(std::string index) { this->_index = index; }
// void ServerCfg::setLocations(std::vector<Location> locations)
// {
// 	this->_locations = locations;
// }

uint16_t		ServerCfg::getPort() { return (this->_port); }
in_addr_t		ServerCfg::getHost() { return (this->_host); }
std::string ServerCfg::getServerName() { return (this->_server_name); }
std::string ServerCfg::getRoot() { return (this->_root); }
sockaddr_in ServerCfg::getServerAddr() { return (this->_server_addr); }
int					ServerCfg::getListenFd() { return (this->_listen_fd); }

// unsigned long					ServerCfg::getClientMaxBodySize() {}
// std::string						ServerCfg::getIndex() {}
// std::vector<Location> ServerCfg::getLocations() {}

// uint16_t	_port;
// in_addr_t _host;
// std::string				 _server_name;
// std::string				 _root;
// struct sockaddr_in _server_addr;
// int								 _listen_fd;
//
// NEXT: for the other props we need conversion functions first. add utils.cpp
// with our own conversion functions for ip-addrs.
void ServerCfg::printCfg() const
{
	std::cout << "printing cfg for server \"" << _server_name << "\":\n"
						<< std::endl;
	std::cout << "  port: " << _port << std::endl;
	std::cout << "  root: " << _root << std::endl;
	std::cout << "  listen_fd: " << _listen_fd << std::endl;
	std::cout << std::endl;
}
