/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv_Utils.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 11:54:27 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/22 13:36:32 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Webserv.hpp"
#include "utils.hpp"

// for memset
#include <cstring>

bool Webserv::_isServerFd(int fd) const
{
  return (_vserverFdMap.find(fd) != _vserverFdMap.end());
}

// TODO: refactor to return a list of possible servers, namely the servers
// listening on this specific fd.
// NOTE: std::map -> std::multimap
// FIXME: This has to be refactor to handle multiple real VServers listening on
// the same FD but differing bny server_name
std::vector<VServer *> Webserv::_getServerByFd(int fd)
{
  std::map< int, std::vector<VServer *> >::iterator it;
  it = _vserverFdMap.find(fd);
  if (it == _vserverFdMap.end())
    return std::vector<VServer *>();
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

// NOTE: this _only_ needs to be done for the default config. all non-default
// config wil have gone through parseCfg where all values will be set.
void Webserv::_initDefaultCfg()
{
  VServerCfg cfg1;
  Route      r;
  cfg1.setServerName("Test_4284");
  cfg1.addInterface("127.0.0.1", 4284);
  cfg1.addRoute(r);
  VServer dsrv1(cfg1);
  _vservers.push_back(dsrv1);
  ++_numOfServers;
}
