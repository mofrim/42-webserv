/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServerCfg.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 08:35:42 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/11 13:46:55 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "VServerCfg.hpp"
#include "utils.hpp"

// --------------------------------=[ OCF ]=-------------------------------- //

// Default Constructor with default values.
// NOTE: do all non-generic setting in ServerSetup
VServerCfg::VServerCfg()
{
  _serverName  = "";
  _maxBodySize = MAX_BODY_SIZE;
  _root        = "./www";
  _initSetDirecs();
}

VServerCfg::VServerCfg(const VServerCfg& o)
{
  _serverName  = o._serverName;
  _interfaces  = o._interfaces;
  _maxBodySize = o._maxBodySize;
  _routes      = o._routes;
  _errPages    = o._errPages;
  _root        = o._root;
  _setDirecs   = o._setDirecs;
}

VServerCfg& VServerCfg::operator=(const VServerCfg& o)
{
  if (this != &o) {
    _serverName  = o._serverName;
    _interfaces  = o._interfaces;
    _maxBodySize = o._maxBodySize;
    _routes      = o._routes;
    _errPages    = o._errPages;
    _root        = o._root;
    _setDirecs   = o._setDirecs;
  }
  return (*this);
}

VServerCfg::~VServerCfg() {}

//// OCF end

void VServerCfg::setServerName(std::string name) { _serverName = name; }

std::string VServerCfg::getServerName() const { return (_serverName); }

void VServerCfg::printCfg() const
{
  Logger::log_msg("  serverName: \"" + _serverName + "\"");
  Logger::log_msg("  maxBodySize: \"" + int2str(_maxBodySize) + "\"");
  Logger::log_msg("  root: \"" + _root + "\"");

  Logger::log_msg("  interfaces:");
  for (std::map<str, std::set<u16> >::const_iterator it = _interfaces.begin();
      it != _interfaces.end();
      ++it)
    Logger::log_msg("    " + it->first + ": " + getSetAsStr(it->second));

  Logger::log_msg("  errPages:");
  for (std::map<e_HTTPStatus, str>::const_iterator it = _errPages.begin();
      it != _errPages.end();
      ++it)
  {
    Logger::log_msg("    - " + int2str(it->first) + ":" + it->second);
  }

  Logger::log_msg("  routes:");
  for (std::map<str, Route>::const_iterator it = _routes.begin();
      it != _routes.end();
      it++)
  {
    const Route& r = it->second;
    Logger::log_msg("   - \"" + it->first + "\": ");
    Logger::log_msg("     + root = " + r.getRoot());
    Logger::log_msg("     + upload = " + r.getUpload());
    Logger::log_msg("     + index = " + r.getIndex());
    Logger::log_msg("     + autoindex = " + bool2str(r.getAutoindex()));
    Logger::log_msg("     + maxBodySize = " + u32ToStr(r.getMaxBodySize()));

    str meths;
    for (std::set<e_Method>::const_iterator it = r.getMethods().begin();
        it != r.getMethods().end();
        ++it)
    {
      meths += meth2str(*it) + " ";
    }
    Logger::log_msg("     + methods = " + meths);

    const std::pair<e_HTTPStatus, str>& redir = r.getRedir();
    Logger::log_msg(
        "     + redir = " + int2str(redir.first) + ":" + redir.second);

    str cgistr;
    for (std::map<str, str>::const_iterator it = r.getCgi().begin();
        it != r.getCgi().end();
        ++it)
      cgistr += it->first + ":" + it->second + ", ";
    Logger::log_msg("     + cgi = " + cgistr);
  }
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

void VServerCfg::addErrPage(e_HTTPStatus s, const str& path)
{
  _errPages[s] = path;
}

str VServerCfg::getErrPage(e_HTTPStatus s) { return _errPages[s]; }

std::map<e_HTTPStatus, str> VServerCfg::getErrPages() const
{
  return _errPages;
}

// FIXME: think about this more later. Do i need? Or can i handle this with good
// default values?
void VServerCfg::_initSetDirecs()
{
  _setDirecs["serverName"]  = false;
  _setDirecs["interfaces"]  = false;
  _setDirecs["maxBodySize"] = false;
  _setDirecs["root"]        = false;
  _setDirecs["errPages"]    = false;
  _setDirecs["routes"]      = false;
}

void VServerCfg::setRoot(str r) { _root = r; }

str VServerCfg::getRoot() const { return _root; }

// TODO ...or do even need this? Is there more to be done here?
bool VServerCfg::checkEnsureCfg()
{

  // add at least default route
  if (_routes.empty())
    _routes["/"] = Route();

  for (std::map<str, Route>::iterator it = _routes.begin(); it != _routes.end();
      ++it)
  {
    Route& r = it->second;

    // ensure that maxBodySize is set for every route
    if (r.getMaxBodySize() == 0)
      r.setMaxBodySize(_maxBodySize);
  }
  return true;
}
