/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv_Utils.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 11:54:27 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/18 17:42:05 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Webserv.hpp"
#include "utils.hpp"

// for memset
#include <cstring>

bool Webserv::_isServerFd(int fd) const
{
  return (_serverFdMap.find(fd) != _serverFdMap.end());
}

// TODO: refactor to return a list of possible servers, namely the servers
// listening on this specific fd.
// NOTE: std::map -> std::multimap
VServer *Webserv::_getServerByFd(int fd)
{
  std::map<int, VServer *>::iterator it;
  it = _serverFdMap.find(fd);
  if (it == _serverFdMap.end())
    return (NULL);
  return (it->second);
}

VServer *Webserv::_getServerByClientFd(int fd)
{
  _printSockname(fd);
  std::map<int, VServer *>::iterator it;
  it = _clientFdServerMap.find(fd);
  if (it == _clientFdServerMap.end())
    return (NULL);
  return (it->second);
}

void Webserv::_printSockname(int sock)
{
  struct sockaddr_in addr;
  socklen_t          addrlen = sizeof(addr);
  if (getsockname(sock, (struct sockaddr *)&addr, &addrlen) == -1)
    throw(WebservRunException("could not getsockname for fd " + int2str(sock)));
  Logger::log_dbg0("getsockname: " + getAddrPortStr4(addr));
}

void Webserv::_addClientToClientFdServerMap(int fd, VServer *srv)
{
  _clientFdServerMap.insert(std::pair<int, VServer *>(fd, srv));
}

// NOTE: this _only_ needs to be done for the default config. all non-default
// config wil have gone through parseCfg where all values will be set.
void Webserv::_initDefaultCfg()
{
  VServer     dsrv;
  sockaddr_in srv_addr;

  dsrv.setServerName(DEFAULT_SRV_NAME);
  dsrv.setPort(DEFAULT_PORT);
  dsrv.setRoot("./www");

  memset(&srv_addr, 0, sizeof(srv_addr));
  srv_addr.sin_family      = AF_INET;
  srv_addr.sin_addr.s_addr = INADDR_ANY;
  srv_addr.sin_port        = htons(DEFAULT_PORT);
  dsrv.setServerAddr(srv_addr);
  dsrv.setHost(INADDR_LOOPBACK);

  _numOfServers = 1;
}

// TODO:
// NEXT: refac this to play together with Socket::bindSocket
// ...seemlingly everthing is tied to the problem that my server still cannot
// bind to multiple interfaces.
void Webserv::_initDefaultCfg2()
{

  // VServer     dsrv2;
  sockaddr_in srv_addr;

  VServerCfg cfg1;

  cfg1.addInterface("0.0.0.0", 4284);
  cfg1.addInterface("0.0.0.0", 4285);
  cfg1.setServerName("Test_4284");
  VServer dsrv1(cfg1);
  dsrv1.setServerName("testsrv_4284");
  dsrv1.setPort(DEFAULT_PORT);
  dsrv1.setRoot("./www");

  memset(&srv_addr, 0, sizeof(srv_addr));
  srv_addr.sin_family      = AF_INET;
  srv_addr.sin_addr.s_addr = INADDR_ANY;
  srv_addr.sin_port        = htons(DEFAULT_PORT);
  dsrv1.setServerAddr(srv_addr);
  dsrv1.setHost(INADDR_ANY);

  // dsrv2.setServerName("testsrv_4285");
  // dsrv2.setPort(4285);
  // dsrv2.setRoot("./www");
  //
  // memset(&srv_addr, 0, sizeof(srv_addr));
  // srv_addr.sin_family      = AF_INET;
  // srv_addr.sin_addr.s_addr = INADDR_ANY;
  // srv_addr.sin_port        = htons(4285);
  // dsrv2.setServerAddr(srv_addr);
  // dsrv2.setHost(INADDR_ANY);

  _servers.push_back(dsrv1);
  // _servers.push_back(dsrv2);

  // _numOfServers = 2;
  _numOfServers = 1;
}
