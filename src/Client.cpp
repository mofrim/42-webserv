/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 20:51:06 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/26 15:40:11 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Epoll.hpp"
#include "Logger.hpp"
#include "VServer.hpp"
#include "Webserv.hpp"
#include "utils.hpp"

#include <cerrno>
#include <sys/epoll.h>
#include <unistd.h>
// --------------------------------=[ OCF ]=-------------------------------- //

Client::Client():
  _clientFd(-1), _timeout(false), _vsrv(NULL), _reqHandler(this),
  _state(CLI_IDLE)
{}

Client::Client(const Client& o) { (void)o; }

Client::Client(
    Webserv *wsrv, int fd, VServer *vsrv, const str& addr, in_port_t port):
  _webserv(wsrv), _clientFd(fd), _addr(addr), _port(ntohs(port)), _vsrvPort(0),
  _timeout(false), _lastActive(time(NULL)), _vsrv(vsrv), _reqHandler(this),
  _state(CLI_IDLE)
{
  _ifaceFdStr = addr + ":" + int2str(_port) + ":fd=" + int2str(fd);
  if (vsrv) {
    _reqHandler.setVsrvName(vsrv->getName());
    _virtual = false;
  }
  else {
    _reqHandler.setVsrvName("__VIRTUAL__");
    _virtual = true;
  }
  this->setLastActive();
}

// NEXT make virtual clients handling really virtual

// not used
Client& Client::operator=(const Client& o)
{
  (void)o;
  return (*this);
}

Client::~Client()
{
  Logger::logDbg0("Destroying client " + _ifaceFdStr);
  close(_clientFd);
}

// ------------------------------=[ END OCF ]=------------------------------ //

void Client::setVsrv(VServer *v)
{
  _vsrv = v;
  if (v != NULL)
    _req.setVsrvName(v->getName());
  else
    _req.setVsrvName("__VIRTUAL__");
}

void Client::setPotentialVsrvs(std::vector<VServer *> vv)
{
  _potentialVsrvs = vv;
}

// This function only exists for generating new Client instances that have not
// yet been assigned to a server, which is the case for clients knocking on the
// door of pure virtual servers
//
// FIXME: avoid this codedup with VServer::addClient
Client *Client::newVirtualCli(Webserv *wsrv, int listenFd)
{
  struct sockaddr_in client_addr;
  socklen_t          client_addr_len = sizeof(client_addr);

  int client_fd =
      accept(listenFd, (struct sockaddr *)&client_addr, &client_addr_len);
  if (client_fd == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      Logger::logErr("accept failed with EAGAIN || WOULDBLOCK");
    else
      Logger::logErr("accept failed: " + getErrnoStr());
  }

  if (setFdNonBlocking(client_fd) == -1) {
    close(client_fd);
    return NULL;
  }

  str     hostname(inAddrToStr(client_addr.sin_addr));
  Client *newCli =
      new Client(wsrv, client_fd, NULL, hostname, client_addr.sin_port);

  Logger::logSrv("__VIRTUAL__",
      "New virtual client connected from " + newCli->getIfaceFdStr());

  newCli->setLastActive();

  return newCli;
}

void Client::handleEvent(u32 ev)
{
  if (ev & (EPOLLIN | EPOLLOUT)) {
    this->setLastActive();
    Logger::logMsg(
        "Got " + Epoll::getEventStr(ev) + " on client " + getIfaceFdStr());
    if (ev & EPOLLIN) {
      _reqHandler.readRequest();
    }
    if (ev & EPOLLOUT) {
      _reqHandler.writeResponse();
    }
  }
}

// extracted routine for evaluating any EPOLLHUP or EPOLLERR events received on
// the pipeFds. there are 3 possible error scenacrios documented in the code
// below. the 4th case is not an error.
void Client::_cgiEvalEpollHupErr(u32 ev)
{
  // 1) parent's write end of the pipe closed prematurely. EPOLLERR can only
  // happen on the writeFd! Always an error as we could not write the whole
  // request body to pipe.
  if (ev & EPOLLERR) {
    Logger::logSrv(
        _vsrv->getName(), "Got EPOLLERR from client " + getIfaceFdStr());
    _state = CLI_CGIKO;
    _req.setStatusCode(HTTP_502);
  }

  else if (ev & EPOLLHUP) {

    // 2) This can only happen to a parent's read-fd! We get EPOLLHUP on the
    // readFd which means the child process closed its stdout while we were
    // still writing the request body. so the child can not have processed the
    // request correctly.
    if (_state == CLI_CGIRW) {
      Logger::logSrv(_vsrv->getName(),
          "EPOLLHUP from client " + getIfaceFdStr() +
              " while still writing to pipe.");
      _req.setStatusCode(HTTP_502);
      _state = CLI_CGIKO;
    }

    // 3) We're done writing but we get an EPOLLHUP on the read fd. If the
    // child's exit status is bad. We have an error. Otherwise we have to keep
    // reading until we get EOF.
    else if (_state == CLI_CGIREAD) {
      if (_req.cgiEvalChildState() > 0)
        _state = CLI_CGIKO;
    }
  }
}

void Client::handleEventCGI(u32 ev, int fd)
{
  str whichFd = _req.cgiIsWriteFd(fd) ? " CGI-Write-FD " : " CGI-Read-FD ";
  Logger::logMsg(
      "Got " + Epoll::getEventStr(ev) + " on" + whichFd + int2str(fd));

  _cgiEvalEpollHupErr(ev);

  switch (_state) {
    case CLI_CGIRW:
      if (ev & EPOLLOUT) {
        this->setLastActive();
        _req.cgiWrite();
      }
      else if (ev & EPOLLIN) {
        this->setLastActive();
        _req.cgiRead();
      }
      if (_state == CLI_CGIOK || _state == CLI_CGIKO ||
          _req.cgiEvalChildState() > 0)
        break;
      else
        return;
    case CLI_CGICDONE:
    case CLI_CGIREAD:
      if (ev & (EPOLLIN | EPOLLHUP)) {
        this->setLastActive();
        _req.cgiRead();
      }
      if (_state == CLI_CGIOK || _state == CLI_CGIKO ||
          _req.cgiEvalChildState() > 0)
        break;
      else
        return;
    default:
      break;
  }

  if (_state == CLI_CGIKO || _state == CLI_CGIOK) {
    if (_state == CLI_CGIOK)
      _req.cgiProcessBody();

    if (_req.cgiEvalChildState() == CHILD_RUNNING)
      _webserv->addCgiPidToKillQueue(_req.cgiGetCpid());

    _req.cgiCleanupFds();
    _req.setStatusFromRespo();
    _req.buildResponse();
    _req.setCGIdone();
  }
}

// -----------------------=[ One-liners, mostly... ]=----------------------- //

Request& Client::getReq() { return _req; }
void     Client::setReq(const Request& r) { _req = r; }

// reset the request object and, if cli is virtual, set reqhandlers server name
// to __VIRTUAL__ as we don't know where the next request is going to.
void Client::resetReq()
{
  _req.reset();

  if (_virtual) {
    _reqHandler.setVsrvName("__VIRTUAL__");
    _req.setVsrvName("__VIRTUAL__");
  }
}

str        Client::getIfaceFdStr() const { return _ifaceFdStr; }
e_CliState Client::getState() const { return _state; }
void       Client::setState(e_CliState s) { _state = s; }
bool       Client::isIdling() const { return _state == CLI_IDLE; }
bool       Client::isReading() const { return _state == CLI_READ; }
bool       Client::isSending() const { return _state == CLI_SEND; }
bool       Client::isDisco() const { return _state == CLI_DISCO; }
bool       Client::isDraining() const { return _state == CLI_DRAIN; }

bool     Client::isTimeout() const { return _timeout; }
void     Client::setFd(int fd) { _clientFd = fd; }
int      Client::getFd() const { return _clientFd; }
void     Client::setLastActive() { _lastActive = time(NULL); }
clock_t  Client::getLastActive() const { return (_lastActive); }
VServer *Client::getVsrv() const { return _vsrv; }
str      Client::getAddr() const { return _addr; }
u16      Client::getPort() const { return _port; }
void     Client::timeout() { _timeout = true; }
bool     Client::isVirtual() const { return _virtual; }

void Client::setVsrvPort(u16 port) { _vsrvPort = port; }

u16 Client::getVsrvPort() const { return _vsrvPort; }

std::vector<VServer *>& Client::getPotentialVsrvs() { return _potentialVsrvs; }

bool Client::isWsrvVsrvName(constr& s) const { return _webserv->isVsrvName(s); }

// --------------------------------=[ CGI ]=-------------------------------- //

void Client::addCgiToEpoll(int fdWrite, int fdRead)
{
  _webserv->addCgiCliToEpoll(this, fdWrite, fdRead);
}

void Client::delCgiFromEpoll(int fd) { _webserv->removeCgiFdFromEpoll(fd); }

bool Client::isCGIing() const
{
  return _state == CLI_CGIREAD || _state == CLI_CGIRW || _state == CLI_CGIKO ||
      _state == CLI_CGIOK || _state == CLI_CGICDONE;
}
