/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 20:51:06 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/26 17:37:43 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Logger.hpp"
#include "VServer.hpp"
#include "utils.hpp"

#include <sys/epoll.h>
#include <unistd.h>

// --------------------------------=[ OCF ]=-------------------------------- //

Client::Client():
  _clientFd(-1), _timeout(false), _vsrv(NULL), _reqHandler(this),
  _state(CLI_IDLE)
{}

Client::Client(const Client& o)
{
  (void)o;
}

Client::Client(int fd, VServer *vsrv, const str& addr, in_port_t port):
  _clientFd(fd), _addr(addr), _port(ntohs(port)), _timeout(false),
  _lastActive(time(NULL)), _vsrv(vsrv), _reqHandler(this), _state(CLI_IDLE)
{
  _remoteInterface = addr + ":" + int2str(_port);
  _reqHandler.setVsrvname(vsrv->getServerName());
}

Client& Client::operator=(const Client& o)
{
  (void)o;
  return (*this);
}

Client::~Client()
{
  Logger::log_dbg0(
      "Client " + _remoteInterface + " closing fd " + int2str(_clientFd));
  close(_clientFd);
}

// ------------------------------=[ END OCF ]=------------------------------ //

void Client::setFd(int fd)
{
  _clientFd = fd;
}
int Client::getFd() const
{
  return _clientFd;
}

void Client::setLastActive()
{
  _lastActive = time(NULL);
}

clock_t Client::getLastActive() const
{
  return (_lastActive);
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

void Client::handleEvent(u32 ev)
{
  if (ev & (EPOLLIN | EPOLLOUT)) {
    this->setLastActive();
    if (ev & EPOLLIN) {
      Logger::log_msg("Got EPOLLIN!");
      _reqHandler.readRequest();
    }
    if (ev & EPOLLOUT) {
      Logger::log_msg("Got EPOLLOUT!");
      _reqHandler.writeResponse();
    }
  }
}

Request& Client::getReq()
{
  return _req;
}

void Client::setReq(const Request& r)
{
  _req = r;
}

void Client::resetReq()
{
  _req.reset();
}

str Client::getRemoteInterface() const
{
  return _remoteInterface;
}

e_CliState Client::getState() const
{
  return _state;
}

void Client::setState(e_CliState s)
{
  _state = s;
}

bool Client::isIdling() const
{
  return _state == CLI_IDLE;
}

bool Client::isReading() const
{
  return _state == CLI_READ;
}

bool Client::isSending() const
{
  return _state == CLI_SEND;
}

bool Client::isDisco() const
{
  return _state == CLI_DISCO;
}

bool Client::isReqComplete() const
{
  return _req.reqComplete();
}

void Client::setReqFinished()
{
  _req.setFinished();
}

bool Client::isTimeout() const
{
  return _timeout;
}
