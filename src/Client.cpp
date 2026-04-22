/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 20:51:06 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/22 03:18:00 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Logger.hpp"
#include "utils.hpp"

#include <unistd.h>

// -- OCF --
Client::Client(): _client_fd(-1)
{}

Client::Client(const Client& other)
{
  if (this != &other) {
    _client_fd   = other._client_fd;
    _last_access = other._last_access;
  }
}

Client::Client(
    int fd, VServer *vsrv, const std::string& hostname, in_port_t port):
  _client_fd(fd), _hostname(hostname), _port(port), _vsrv(vsrv),
  _last_access(clock())
{
  // FIXME: remove when _port is used somewhere
  (void)_port;
}

Client& Client::operator=(const Client& other)
{
  (void)other;
  return (*this);
}

Client::~Client()
{
  Logger::log_dbg0("Client closing fd " + int2str(_client_fd));
  close(_client_fd);
}
// -- OCF end --

void Client::setFd(int fd)
{
  _client_fd = fd;
}
int Client::getFd() const
{
  return (_client_fd);
}

void Client::setLastAccess(clock_t t)
{
  _last_access = t;
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
