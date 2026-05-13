/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServer_Utils.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 12:11:11 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/13 13:18:07 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Socket.hpp"
#include "VServer.hpp"
#include "utils.hpp"

#include <iostream>

// -----------------------------=[ Exceptions ]=----------------------------- //

VServer::ServerInitException::ServerInitException(const std::string& msg):
  std::runtime_error("ServerInitException: " + msg)
{}

VServer::ServerException::ServerException(const std::string& msg):
  std::runtime_error("ServerException: " + msg)
{}

// -----------------=[ Server methods of minor importance ]=----------------- //

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

// FIXME: possibly remove code-dup with Route::printCfg
void VServer::printCfg() const
{
  Logger::logCfg("  server_name: \"" + _srvName + "\"");
  Logger::logCfg("  listen_fds: " + getSetAsStr(_listenFds));

  Logger::logCfg("  active listen interfaces:");
  for (std::vector<t_vsrvInterface>::const_iterator it =
           _activeInterfaces.begin();
      it != _activeInterfaces.end();
      it++)
    Logger::logCfg(
        "    " + it->ip + "(" + it->cname + "):" + getSetAsStr(it->portFd));

  Logger::logCfg("  maxBodySize: \"" + int2str(_maxBodySize) + "\"");
  Logger::logCfg("  root: \"" + _root + "\"");

  Logger::logCfg("  errPages:");
  for (std::map<e_HTTPStatus, str>::const_iterator it = _errPages.begin();
      it != _errPages.end();
      ++it)
  {
    Logger::logCfg("    - " + int2str(it->first) + ":" + it->second);
  }

  Logger::logCfg("  routes:");
  for (std::map<str, Route>::const_iterator it = _routes.begin();
      it != _routes.end();
      it++)
  {
    const Route& r = it->second;
    Logger::logCfg("   - \"" + it->first + "\": ");
    Logger::logCfg("     + path = " + r.getPath());
    Logger::logCfg("     + root = " + r.getRoot());
    Logger::logCfg("     + upload = " + r.getUpload());
    Logger::logCfg("     + index = " + r.getIndex());
    Logger::logCfg("     + autoindex = " + bool2str(r.getAutoindex()));
    Logger::logCfg("     + maxBodySize = " + u32ToStr(r.getMaxBodySize()));

    str meths;
    for (std::set<e_Method>::const_iterator it = r.getMethods().begin();
        it != r.getMethods().end();
        ++it)
    {
      meths += meth2str(*it) + " ";
    }
    Logger::logCfg("     + methods = " + meths);

    const std::pair<e_HTTPStatus, str>& redir = r.getRedir();
    Logger::logCfg(
        "     + redir = " + int2str(redir.first) + ":" + redir.second);

    str cgistr;
    for (std::map<str, str>::const_iterator it = r.getCgi().begin();
        it != r.getCgi().end();
        ++it)
      cgistr += it->first + ":" + it->second + ", ";
    Logger::logCfg("     + cgi = " + cgistr);
  }
}

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

str VServer::getErrPage(e_HTTPStatus c)
{
  if (_errPages.find(c) != _errPages.end())
    return _errPages[c];
  return "";
}

void VServer::setMaxBodySize(u32 mbs) { _maxBodySize = mbs; }

u32 VServer::getMaxBodySize() const { return _maxBodySize; }

const std::set<int>& VServer::getListenFds() const { return _listenFds; }

const std::set<u16>& VServer::getPorts() const { return _ports; }

void VServer::setRoutes(const std::map<str, Route>& r) { _routes = r; }

const std::map<str, Route>& VServer::getRoutes() const { return _routes; }

std::string VServer::getName() const { return (_srvName); }

bool VServer::isInitFailed() const { return (_setupFailed); }

void VServer::setServerName(std::string name) { _srvName = name; }

void VServer::setSetupFailed() { _setupFailed = true; }

void VServer::setRoot(constr& s) { _root = s; }

constr& VServer::getRoot() const { return _root; }
