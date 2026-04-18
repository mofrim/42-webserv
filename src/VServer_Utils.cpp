/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServer_Utils.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 12:11:11 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/18 16:06:03 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "VServer.hpp"
#include "utils.hpp"

#include <iostream>

////////////////////////////////////////////////////////////////////////////////
/// Server methods of minor importance

// the getters
uint16_t VServer::getPort() const
{
  return (_port);
}
in_addr_t VServer::getHost() const
{
  return (_host);
}
std::string VServer::getServerName() const
{
  return (_server_name);
}
std::string VServer::getRoot() const
{
  return (_root);
}
sockaddr_in VServer::getServerAddr() const
{
  return (_server_addr);
}
int VServer::getListenFd() const
{
  return (_listen_fd);
}
const VServerCfg *VServer::getCfg() const
{
  return (&_cfg);
}
bool VServer::getSetupFailed() const
{
  return (_setupFailed);
}

// the setters
void VServer::setPort(uint16_t port)
{
  _port = port;
}
void VServer::setHost(in_addr_t host)
{
  _host = host;
}
void VServer::setServerName(std::string name)
{
  _server_name = name;
}
void VServer::setRoot(std::string root)
{
  _root = root;
}
void VServer::setServerAddr(sockaddr_in server_addr)
{
  this->_server_addr = server_addr;
}
void VServer::setListenFd(int listen_fd)
{
  _listen_fd = listen_fd;
}
void VServer::setSetupFailed()
{
  _setupFailed = true;
}

void VServer::printCfg() const
{
  struct in_addr host_addr;
  host_addr.s_addr = htonl(_host);
  Logger::log_msg("  server_name: \"" + _server_name + "\"");
  Logger::log_msg("  port: " + int2str(_port));
  Logger::log_msg("  root: " + _root);
  Logger::log_msg("  listen_fd: " + int2str(_listen_fd));
  Logger::log_msg("  host: " + inAddrToStr(host_addr));
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
