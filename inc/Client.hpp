/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 20:50:12 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/25 19:44:08 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "WsrvLib.hpp"

#include <ctime>
#include <netinet/in.h>
#include <vector>

class VServer;

class Client {
  private:
    int      _client_fd;
    str      _addr;
    u16      _port;
    VServer *_vsrv;
    bool     _timeout;

    std::vector<VServer *> _potentialVsrvs;

    time_t _last_access;

    Client(const Client& other);
    Client& operator=(const Client& other);

  public:
    Client();
    ~Client();

    Client(int fd, VServer *vsrv, const str& addr, in_port_t port);

    void setFd(int fd);
    int  getFd() const;

    void   setLastAccess();
    time_t getLastAccess() const;

    str getAddr() const;
    u16 getPort() const;

    void timeout();

    void     setVsrv(VServer *v);
    VServer *getVsrv() const;

    void                    setPotentialVsrvs(std::vector<VServer *> vv);
    std::vector<VServer *>& getPotentialVsrvs();

    static Client *newCliServerless(int listenFd);
};
