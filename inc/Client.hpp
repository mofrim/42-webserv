/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 20:50:12 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/27 00:11:28 by fmaurer          ###   ########.fr       */
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
    str    _ifaceFdStr;
    bool   _timeout;
    time_t _lastActive;

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
    void       setLastActive();
    time_t     getLastActive() const;
    str        getAddr() const;
    u16        getPort() const;
    void       timeout();
    void       setVsrv(VServer *v);
    VServer   *getVsrv() const;
    Request&   getReq();
    void       setReq(const Request& r);
    void       resetReq();
    str        getIfaceFdStr() const;
    e_CliState getState() const;
    void       setState(e_CliState s);
    void       setReqFinished();

    bool isReqComplete() const;
    bool isTimeout() const;

    bool isIdling() const;
    bool isReading() const;
    bool isSending() const;
    bool isDisco() const;

    bool reqHasHostHeader();

    void                    setPotentialVsrvs(std::vector<VServer *> vv);
    std::vector<VServer *>& getPotentialVsrvs();

    static Client *newCliServerless(int listenFd);

    void handleEvent(u32 ev);
    void handleEventServerless(u32 ev);
};
