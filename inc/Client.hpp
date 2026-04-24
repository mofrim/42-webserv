/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 20:50:12 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/24 20:36:17 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <ctime>
#include <netinet/in.h>
#include <string>
#include <vector>

class VServer;

class Client {
  private:
    int         _client_fd;
    std::string _hostname;
    in_port_t   _port;
    VServer    *_vsrv;

    std::vector<VServer *> _potentialVsrvs;

    time_t _last_access;

  public:
    Client();
    Client(const Client& other);
    Client& operator=(const Client& other);
    ~Client();

    Client(int fd, VServer *vsrv, const std::string& hostname, in_port_t port);

    void setFd(int fd);
    int  getFd() const;

    void   setLastAccess(time_t t);
    time_t getLastAccess() const;

    void     setVsrv(VServer *v);
    VServer *getVsrv() const;

    void                    setPotentialVsrvs(std::vector<VServer *> vv);
    std::vector<VServer *>& getPotentialVsrvs();

    static Client *newCliServerless(int listenFd);
};
