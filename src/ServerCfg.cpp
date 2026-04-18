/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerCfg.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 08:35:42 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/18 13:51:12 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerCfg.hpp"
#include "utils.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>

// --------------------------------=[ OCF ]=-------------------------------- //

// Default Constructor with default values.
// NOTE: do all non-generic setting in ServerSetup
ServerCfg::ServerCfg()
{
  _port        = 0;
  _host        = 0;
  _server_name = "";
  _root        = "";
  memset(&_server_addr, 0, sizeof(_server_addr));
}

ServerCfg::ServerCfg(const ServerCfg& o)
{
  _port        = o._port;
  _host        = o._host;
  _server_name = o._server_name;
  _root        = o._root;
  _server_addr = o._server_addr;
  _interfaces  = o._interfaces;
}

ServerCfg& ServerCfg::operator=(const ServerCfg& o)
{
  if (this != &o) {
    _port        = o._port;
    _host        = o._host;
    _server_name = o._server_name;
    _root        = o._root;
    _server_addr = o._server_addr;
    _interfaces  = o._interfaces;
  }
  return (*this);
}

ServerCfg::~ServerCfg()
{}

//// OCF end

void ServerCfg::setServerName(std::string name)
{
  _server_name = name;
}
void ServerCfg::setRoot(std::string root)
{
  _root = root;
}
void ServerCfg::setDefaultFile(std::string default_file)
{
  this->_default_file = default_file;
}
void ServerCfg::setPort(uint16_t port)
{
  _port = port;
}
void ServerCfg::setHost(in_addr_t host)
{
  _host = host;
}
void ServerCfg::setServerAddr(sockaddr_in server_addr)
{
  this->_server_addr = server_addr;
}

// void ServerCfg::setClientMaxBodySize(unsigned long max_body_size)
// {
// 	this->_client_max_body_size = max_body_size;
// }
// void ServerCfg::setLocations(std::vector<Location> locations)
// {
// 	this->_locations = locations;
// }

uint16_t ServerCfg::getPort() const
{
  return (_port);
}
in_addr_t ServerCfg::getHost() const
{
  return (_host);
}
std::string ServerCfg::getServerName() const
{
  return (_server_name);
}
std::string ServerCfg::getRoot() const
{
  return (_root);
}
sockaddr_in ServerCfg::getServerAddr() const
{
  return (_server_addr);
}
std::string ServerCfg::getDefaultFile() const
{
  return (_default_file);
}

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
  std::cout << "  host: " << inAddrToStr(host_addr) << std::endl;
}

// silently fails if addr:port pair already exists
void ServerCfg::addInterface(const str& addr, u16 port)
{
  _interfaces[addr].insert(port);
}

void ServerCfg::addInterfaces(std::map<str, std::set<u16> > interfaces)
{
  for (std::map<str, std::set<u16> >::iterator it = interfaces.begin();
      it != interfaces.end();
      it++)
    _interfaces[it->first] = it->second;
}

std::map< str, std::set<u16> >& ServerCfg::getInterfaces()
{
  return _interfaces;
}

// FIXME: is it okay to return a reference here? or should it be a complete
// object?
std::set<u16>& ServerCfg::getPorts(const str& addr)
{
  return _interfaces[addr];
}
