/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 23:11:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/26 10:18:47 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLL_HPP
#define EPOLL_HPP

#include "VServer.hpp"

#include <sys/epoll.h>
#include <vector>

// max num of events epoll will manage in one epoll_wait loop
#define MAX_EVENTS 1000
#define EPOLL_TIMEOUT_MS 500

class Epoll {
  private:
    int                             _epoll_fd;
    int                             _nfds;
    std::vector<struct epoll_event> _ev;
    struct epoll_event              _events[MAX_EVENTS];

  public:
    Epoll();
    Epoll(const Epoll& other);
    Epoll& operator=(const Epoll& other);
    ~Epoll();

    void setup(const std::vector<VServer>& servers);
    int  wait();
    void addClient(int cfd, u32 event = EPOLLIN);
    void modifyClient(int cfd, uint32_t events);
    void removeClient(int cfd);
    void closeEpollFd();

    int        getEpollFd() const;
    int        getEventFd(int event_idx) const;
    u32        getEvent(int event_idx) const;
    static str getEventStr(const uint32_t& ev);

    void printReadylist() const;

    class EpollException: public std::runtime_error {
      public:
        EpollException(const str& msg);
    };
};

#endif
