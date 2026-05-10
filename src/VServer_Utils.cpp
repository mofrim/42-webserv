/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServer_Utils.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 12:11:11 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/10 23:02:04 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Socket.hpp"
#include "VServer.hpp"
#include "utils.hpp"

#include <iostream>

////////////////////////////////////////////////////////////////////////////////
/// Server methods of minor importance

std::string VServer::getName() const { return (_srvName); }

bool VServer::isInitFailed() const { return (_setupFailed); }

void VServer::setServerName(std::string name) { _srvName = name; }

void VServer::setSetupFailed() { _setupFailed = true; }

void VServer::printClients()
{
  Logger::log_srv(_srvName, "Printing Clients:");
  if (_clients.empty()) {
    Logger::log_msg("-> Server has got no clients");
    return;
  }
  for (std::map<int, Client *>::iterator it = _clients.begin();
      it != _clients.end();
      it++)
  {
    std::cout << "  fd: " << it->second->getFd() << std::endl;
  }
}

// returns true if fd is a member of _clients belonging to this server.
bool VServer::isValidClientFd(int fd)
{
  return (_clients.find(fd) != _clients.end());
}

const std::set<int>& VServer::getListenFds() const { return _listenFds; }

const std::set<u16>& VServer::getPorts() const { return _ports; }

void VServer::setRoutes(const std::map<str, Route>& r) { _routes = r; }

const std::map<str, Route>& VServer::getRoutes() const { return _routes; }

void VServer::printCfg() const
{
  Logger::log_msg("  server_name: \"" + _srvName + "\"");
  Logger::log_msg("  listen_fds: " + getSetAsStr(_listenFds));
  Logger::log_msg("  active listen interfaces:");
  for (std::vector<t_vsrvInterface>::const_iterator it =
           _activeInterfaces.begin();
      it != _activeInterfaces.end();
      it++)
    Logger::log_msg(
        "    " + it->ip + "(" + it->cname + "):" + getSetAsStr(it->portFd));
  Logger::log_msg("  routes:");
  for (std::map<str, Route>::const_iterator it = _routes.begin();
      it != _routes.end();
      it++)
  {
    Logger::log_msg("   - \"" + it->first + "\": ");
    Logger::log_msg("     + root = " + it->second.getRoot());
    Logger::log_msg("     + index = " + it->second.getIndex());
    Logger::log_msg(
        "     + autoindex = " + bool2str(it->second.getAutoindex()));
  }
}

void VServer::setMaxBodySize(u32 mbs) { _maxBodySize = mbs; }

u32 VServer::getMaxBodySize() const { return _maxBodySize; }

// @return true, if the given fd is a virtual fd, meaning, the fd will be added
// to epoll interest-list by the first server who also bound it.
bool VServer::isVirtualFd(int fd) const
{
  return _virtualFds.find(fd) != _virtualFds.end();
}

// add an iface to _activeInterfaces list
void VServer::_addActiveIface(t_AddrinfoReturn ar, u16 port)
{
  t_vsrvInterface vif = {
      .ip     = ar.ip,
      .cname  = ar.cname,
      .portFd = std::set< std::pair<u16, int> >()};
  vif.portFd.insert(std::make_pair(port, ar.fd));

  std::vector<t_vsrvInterface>::iterator it = _activeInterfaces.begin();
  while (it != _activeInterfaces.end()) {
    if (*it == vif) {
      it->portFd.insert(*vif.portFd.begin());
      return;
    }
    ++it;
  }
  _activeInterfaces.push_back(vif);
}
