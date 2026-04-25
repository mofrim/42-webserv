/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 20:51:06 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/25 19:44:27 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Logger.hpp"
#include "utils.hpp"

#include <unistd.h>

// --------------------------------=[ OCF ]=-------------------------------- //

Client::Client(): _client_fd(-1), _timeout(false)
{}

Client::Client(const Client& o)
{
  if (this != &o) {
    _client_fd   = o._client_fd;
    _last_access = o._last_access;
  }
}

Client::Client(int fd, VServer *vsrv, const str& addr, in_port_t port):
  _client_fd(fd), _addr(addr), _port(ntohs(port)), _vsrv(vsrv), _timeout(false),
  _last_access(time(NULL))
{}

Client& Client::operator=(const Client& o)
{
  (void)o;
  return (*this);
}

Client::~Client()
{
  Logger::log_dbg0("Client " + _addr + ":" + int2str(_port) + " closing fd " +
      int2str(_client_fd));
  close(_client_fd);
}

// ------------------------------=[ END OCF ]=------------------------------ //

void Client::setFd(int fd)
{
  _client_fd = fd;
}
int Client::getFd() const
{
  return (_client_fd);
}

void Client::setLastAccess()
{
  _last_access = time(NULL);
}

clock_t Client::getLastAccess() const
{
  return (_last_access);
}

void Client::setVsrv(VServer *v)
{
  _vsrv = v;
}

VServer *Client::getVsrv() const
{
  return _vsrv;
}

void Client::setPotentialVsrvs(std::vector<VServer *> vv)
{
  _potentialVsrvs = vv;
}

std::vector<VServer *>& Client::getPotentialVsrvs()
{
  return _potentialVsrvs;
}

// This function only exists for generating new Client instances that have not
// yet been assigned to a server, which is the case for clients knocking on the
// door of pure virtual servers
//
// FIXME: avoid this codedup with VServer::addClient
Client *Client::newCliServerless(int listenFd)
{
  struct sockaddr_in client_addr;
  socklen_t          client_addr_len = sizeof(client_addr);

  int client_fd =
      accept(listenFd, (struct sockaddr *)&client_addr, &client_addr_len);
  if (client_fd == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      Logger::log_err("accept failed with EAGAIN || WOULDBLOCK");
    else
      Logger::log_err("accept failed: " + getErrStr());
  }
  Logger::log_srv(
      "ServerLess", "Client connected from " + getAddrPortStr4(client_addr));

  if (setFdNonBlocking(client_fd) == -1) {
    close(client_fd);
    return NULL;
  }

  std::string hostname(inAddrToStr(client_addr.sin_addr));
  Client *newCli = new Client(client_fd, NULL, hostname, client_addr.sin_port);
  return newCli;
}

str Client::getAddr() const
{
  return _addr;
}

u16 Client::getPort() const
{
  return _port;
}

void Client::timeout()
{
  _timeout = true;
}
