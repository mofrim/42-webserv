/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv_Utils.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 11:54:27 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/27 20:29:12 by fmaurer          ###   ########.fr       */
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

std::vector<VServer *> Webserv::_getServersByFd(int fd)
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
  cfg1.setServerName("virtual1");
  cfg1.addInterface("127.0.0.1", 1111);
  cfg1.addRoute(r);
  VServer dsrv1(cfg1);

  VServerCfg cfg2;
  cfg2.setServerName("virtual1");
  cfg2.addInterface("localhost", 1111);
  cfg2.addRoute(r);
  VServer dsrv2(cfg2);

  _vservers.push_back(dsrv1);
  _vservers.push_back(dsrv2);
}
