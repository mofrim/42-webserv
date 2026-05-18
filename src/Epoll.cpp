/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 23:12:17 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/18 10:58:40 by fmaurer          ###   ########.fr       */
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

  // arg for epoll_create only has to be a positive number, so...
  _epoll_fd = epoll_create(42);
  if (_epoll_fd == -1)
    throw(EpollException("epoll_create failed"));

  // if (setFdCloexec(_epoll_fd) == -1)
  //   throw EpollException("Could not set FD_CLOEXEC on _epoll_fd");

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

str Epoll::_getEventStr(const uint32_t& ev) const
{
  if (ev & EPOLLIN)
    return str("EPOLLIN");
  if (ev & EPOLLOUT)
    return str("EPOLLOUT");
  return str("UNKNOWN EVENT");
}

void Epoll::printReadylist() const
{
  Logger::logDbg2("epoll_wait returned nfds = " + int2str(_nfds));
  if (_nfds > 0) {
    Logger::logDbg2("Printing Events in the epoll-ready-list:");
    for (int i = 0; i < _nfds; i++)
      Logger::logDbg2("  fd: " + int2str(_events[i].data.fd) +
          ", event: " + _getEventStr(_events[i].events));
  }
}

// -----------------------------=[ Exceptions ]=----------------------------- //

Epoll::EpollException::EpollException(const str& msg):
  std::runtime_error("EpollException: " + msg)
{}
