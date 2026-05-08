/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServerCfg.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 08:35:42 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/08 17:20:31 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VServerCfg.hpp"
#include "utils.hpp"

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>

// --------------------------------=[ OCF ]=-------------------------------- //

// Default Constructor with default values.
// NOTE: do all non-generic setting in ServerSetup
VServerCfg::VServerCfg()
{
  _serverName  = "";
  _maxBodySize = MAX_BODY_SIZE;
}

VServerCfg::VServerCfg(const VServerCfg& o)
{
  _serverName  = o._serverName;
  _interfaces  = o._interfaces;
  _maxBodySize = o._maxBodySize;
  _routes      = o._routes;
}

VServerCfg& VServerCfg::operator=(const VServerCfg& o)
{
  if (this != &o) {
    _serverName  = o._serverName;
    _interfaces  = o._interfaces;
    _maxBodySize = o._maxBodySize;
    _routes      = o._routes;
  }
  return (*this);
}

VServerCfg::~VServerCfg() {}

//// OCF end

void VServerCfg::setServerName(std::string name) { _serverName = name; }

std::string VServerCfg::getServerName() const { return (_serverName); }

void VServerCfg::printCfg() const
{
  std::cout << "  server_name: \"" << _serverName << "\"" << std::endl;
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

const std::map< str, std::set<u16> >& VServerCfg::getInterfaces() const
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

void VServerCfg::setRoutes(const std::map<str, Route>& routes)
{
  _routes = routes;
}

const std::map<str, Route>& VServerCfg::getRoutes() const { return _routes; }

// add a route to _routes map. using the path (aka location) as key.
void VServerCfg::addRoute(const Route& r) { _routes[r.getPath()] = r; }

void VServerCfg::setMaxBodySize(u32 mbs) { _maxBodySize = mbs; }

u32 VServerCfg::getMaxBodySize() const { return _maxBodySize; }
