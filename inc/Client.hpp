/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 20:50:12 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/26 16:25:20 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Request.hpp"
#include "RequestHandler.hpp"
#include "WsrvLib.hpp"

#include <ctime>
#include <netinet/in.h>
#include <vector>

class VServer;

// the states of the client machine
typedef enum { CLI_READ, CLI_SEND, CLI_IDLE, CLI_DISCO } e_CliState;

class Client {
  private:
    int    _clientFd;
    str    _addr;
    u16    _port;
    str    _remoteInterface;
    bool   _timeout;
    time_t _last_access;

    Request _req;

    VServer               *_vsrv;
    std::vector<VServer *> _potentialVsrvs;

    RequestHandler _reqHandler;

    Client(const Client& other);
    Client& operator=(const Client& other);

    e_CliState _state;

  public:
    Client();
    ~Client();

    Client(int fd, VServer *vsrv, const str& addr, in_port_t port);

    void       setFd(int fd);
    int        getFd() const;
    void       setLastAccess();
    time_t     getLastAccess() const;
    str        getAddr() const;
    u16        getPort() const;
    void       timeout();
    void       setVsrv(VServer *v);
    VServer   *getVsrv() const;
    Request&   getReq();
    void       setReq(const Request& r);
    void       resetReq();
    str        getRemoteInterface() const;
    e_CliState getState() const;
    void       setState(e_CliState s);
    void       setReqFinished();

    bool isReqComplete() const;

    bool isIdling() const;
    bool isReading() const;
    bool isSending() const;
    bool isDisco() const;

    void                    setPotentialVsrvs(std::vector<VServer *> vv);
    std::vector<VServer *>& getPotentialVsrvs();

    static Client *newCliServerless(int listenFd);

    void handleEvent(u32 ev);
};
