/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServer_Utils.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 12:11:11 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/21 13:07:07 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "VServer.hpp"
#include "utils.hpp"

#include <iostream>

////////////////////////////////////////////////////////////////////////////////
/// Server methods of minor importance

std::string VServer::getServerName() const
{
  return (_server_name);
}

bool VServer::getSetupFailed() const
{
  return (_setupFailed);
}

void VServer::setServerName(std::string name)
{
  _server_name = name;
}

void VServer::setSetupFailed()
{
  _setupFailed = true;
}

void VServer::printClients()
{
  Logger::log_srv(_server_name, "Printing Clients:");
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

const std::set<int>& VServer::getListenFds() const
{
  return _listen_fds;
}

const std::set<u16>& VServer::getPorts() const
{
  return _ports;
}

void VServer::setRoutes(const std::map<str, Route>& r)
{
  _routes = r;
}

const std::map<str, Route>& VServer::getRoutes() const
{
  return _routes;
}

void VServer::printCfg() const
{
  Logger::log_msg("  server_name: \"" + _server_name + "\"");
  Logger::log_msg("  listen_fds: " + getSetAsStr(_listen_fds));
  Logger::log_msg("  active listen interfaces:");
  for (std::map< str, std::set<u16> >::const_iterator it =
           _activeAddrPortPairs.begin();
      it != _activeAddrPortPairs.end();
      it++)
    Logger::log_msg("    " + it->first + ":" + getSetAsStr(it->second));
  Logger::log_msg("  routes:");
  for (std::map<str, Route>::const_iterator it = _routes.begin();
      it != _routes.end();
      it++)
  {
    Logger::log_msg("   - \"" + it->first + "\": ");
    Logger::log_msg("     + root = " + it->second.getRoot());
    Logger::log_msg("     + defaultFile = " + it->second.getDefaultFile());
    Logger::log_msg(
        "     + autoindex = " + bool2str(it->second.getAutoindex()));
  }
}
