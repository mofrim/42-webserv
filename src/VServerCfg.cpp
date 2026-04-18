/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServerCfg.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 08:35:42 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/18 15:54:14 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VServerCfg.hpp"
#include "utils.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>

// --------------------------------=[ OCF ]=-------------------------------- //

// Default Constructor with default values.
// NOTE: do all non-generic setting in ServerSetup
VServerCfg::VServerCfg()
{
  _port        = 0;
  _host        = 0;
  _server_name = "";
  _root        = "";
  memset(&_server_addr, 0, sizeof(_server_addr));
}

VServerCfg::VServerCfg(const VServerCfg& o)
{
  _port        = o._port;
  _host        = o._host;
  _server_name = o._server_name;
  _root        = o._root;
  _server_addr = o._server_addr;
  _interfaces  = o._interfaces;
}

VServerCfg& VServerCfg::operator=(const VServerCfg& o)
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

VServerCfg::~VServerCfg()
{}

//// OCF end

void VServerCfg::setServerName(std::string name)
{
  _server_name = name;
}
void VServerCfg::setRoot(std::string root)
{
  _root = root;
}
void VServerCfg::setDefaultFile(std::string default_file)
{
  this->_default_file = default_file;
}
void VServerCfg::setPort(uint16_t port)
{
  _port = port;
}
void VServerCfg::setHost(in_addr_t host)
{
  _host = host;
}
void VServerCfg::setServerAddr(sockaddr_in server_addr)
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

uint16_t VServerCfg::getPort() const
{
  return (_port);
}
in_addr_t VServerCfg::getHost() const
{
  return (_host);
}
std::string VServerCfg::getServerName() const
{
  return (_server_name);
}
std::string VServerCfg::getRoot() const
{
  return (_root);
}
sockaddr_in VServerCfg::getServerAddr() const
{
  return (_server_addr);
}
std::string VServerCfg::getDefaultFile() const
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
void VServerCfg::printCfg() const
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
void VServerCfg::addInterface(const str& addr, u16 port)
{
  _interfaces[addr].insert(port);
}

void VServerCfg::addInterfaces(std::map<str, std::set<u16> > interfaces)
{
  for (std::map<str, std::set<u16> >::iterator it = interfaces.begin();
      it != interfaces.end();
      it++)
    _interfaces[it->first] = it->second;
}

// give access to _interfaces... prly we want this this at least const-ref
// FIXME:
const std::map< str, std::set<u16> >& VServerCfg::getInterfaces()
{
  return _interfaces;
}

// FIXME: is it okay to return a reference here? or should it be a complete
// object?
const std::set<u16>& VServerCfg::getPorts(const str& addr)
{
  return _interfaces[addr];
}

// delete a port from an interface.
// @return int 0 if sth was erased, -1 otherwise
int VServerCfg::delPort(const str& interface, u16 port)
{
  if (_interfaces[interface].erase(port) == 0)
    return -1;
  return 0;
}

// delete interface
// @return int 0 if sth was erased, -1 otherwise
int VServerCfg::delInterface(const str& interface)
{
  if (_interfaces.erase(interface) == 0)
    return -1;
  return 0;
}
