/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 23:12:17 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/26 23:08:57 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Epoll.hpp"
#include "Logger.hpp"
#include "utils.hpp"

#include <cstring>
#include <unistd.h>

// --------------------------------=[ OCF ]=-------------------------------- //

Epoll::Epoll(): _nfds(0) {}

Epoll::Epoll(const Epoll& other) { (void)other; }

Epoll& Epoll::operator=(const Epoll& other)
{
  (void)other;
  return (*this);
}

Epoll::~Epoll() {}

// ------------------------------=[ methods ]=------------------------------ //

// setup epoll to watch all server FDs
//
// we first need to count fds, because every vserver can listen on multiple FDs
void Epoll::setup(const std::vector<VServer>& vservers)
{
  std::set<int> listenFds;

  // A small but very important step in the program: create the epoll fd! This
  // will initialize evreything epoll-related on kernel level. As this fd should
  // not leak to any child processes FD_CLOEXEC is being set.
  _epoll_fd = epoll_create1(EPOLL_CLOEXEC);

  if (_epoll_fd == -1)
    throw(EpollException("epoll_create failed"));

  for (size_t k = 0; k < vservers.size(); k++) {
    listenFds = vservers[k].getListenFds();
    for (std::set<int>::iterator itListen = listenFds.begin();
        itListen != listenFds.end();
        itListen++)
    {
      if (vservers[k].isVirtualFd(*itListen))
        continue;
      struct epoll_event e = {.events = EPOLLIN, .data = {.fd = *itListen}};
      _ev.push_back(e);

      if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, *itListen, &_ev.back()) == -1) {
        close(_epoll_fd);
        throw(EpollException("epoll_ctl failed"));
      }
    }
  }
}

// a simple wrapper for our one and only epoll_wait call. stores the number of
// ready fds in _nfds vor usage in printEvents f.ex.
int Epoll::wait()
{
  _nfds = epoll_wait(_epoll_fd, _events, MAX_EVENTS, EPOLL_TIMEOUT_MS);
  return (_nfds);
}

// Add a client's fd to epoll interest list
void Epoll::addClient(int cfd, u32 event)
{
  struct epoll_event client_ev;
  client_ev.events  = event;
  client_ev.data.fd = cfd;
  if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, cfd, &client_ev) == -1) {
    Logger::logErr("epoll_ctl failed");
    close(cfd);
    throw EpollException("adding client to epoll failed");
  }
}

// this is used as an external interface to modify the listening event for a
// client. After processed request modify to EPOLLIN | EPOLLOUT. after the
// response again only to EPOLLIN
void Epoll::modifyClient(int cfd, uint32_t events)
{
  struct epoll_event ev;
  ev.events  = events;
  ev.data.fd = cfd;
  if (epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, cfd, &ev) == -1) {
    throw EpollException("epoll_ctl: EPOLL_CTL_MOD failed");
  }
}

// First delete a client's fd from the epoll interest list and then close it.
// the other way round epoll_ctl fails.
void Epoll::removeClient(int cfd)
{
  if (epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, cfd, 0) == -1)
    throw EpollException("removing clientFd " + int2str(cfd) +
        " from epoll failed with: " + getErrnoStr());
}

// wrapper for closing the epoll fd.
void Epoll::closeEpollFd()
{
  Logger::logWarn("closing epoll fd");
  close(_epoll_fd);
}

int Epoll::getEpollFd() const { return (_epoll_fd); }

int Epoll::getEventFd(int event_idx) const
{
  return (_events[event_idx].data.fd);
}

u32 Epoll::getEvent(int event_idx) const { return (_events[event_idx].events); }

// print all components of a epoll event returned by epoll_wait. Supported
// events: EPOLLIN, EPOLLOUT, EPOLLERR, EPOLLHUP
str Epoll::getEventStr(const uint32_t& ev)
{
  str ret;

  if (ev & EPOLLIN)
    ret += str("EPOLLIN ");
  if (ev & EPOLLOUT)
    ret += str("EPOLLOUT ");
  if (ev & EPOLLERR)
    ret += str("EPOLLERR ");
  if (ev & EPOLLHUP)
    ret += str("EPOLLHUP ");
  return strip(ret);
}

// debugging output of the readylist after a epoll_wait return
void Epoll::printReadylist() const
{
  Logger::logDbg2("epoll_wait returned nfds = " + int2str(_nfds));
  if (_nfds > 0) {
    Logger::logDbg2("Printing Events in the epoll-ready-list:");
    for (int i = 0; i < _nfds; i++)
      Logger::logDbg2("  fd: " + int2str(_events[i].data.fd) +
          ", event: " + getEventStr(_events[i].events));
  }
}

// -----------------------------=[ Exceptions ]=----------------------------- //

Epoll::EpollException::EpollException(const str& msg):
  std::runtime_error("EpollException: " + msg)
{}
