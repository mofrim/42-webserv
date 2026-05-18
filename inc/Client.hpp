/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 20:50:12 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/18 19:56:22 by fmaurer          ###   ########.fr       */
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
class Webserv;

typedef enum {
  CLI_READ,
  CLI_SEND,
  CLI_IDLE,
  CLI_DISCO,
  CLI_DISCO_CGI,
  CLI_DRAIN,
  CLI_CGIRW,
  CLI_CGIREAD,
  CLI_CGICDONE,
  CLI_CGIKO,
  CLI_CGIOK
} e_CliState;

class Client {
  private:
    // only for CGI i had to add this >:(
    Webserv *_webserv;

    int    _clientFd;
    str    _addr;
    u16    _port;
    u16    _vsrvPort;
    str    _ifaceFdStr;
    bool   _timeout;
    time_t _lastActive;
    bool   _virtual;

    Request _req;

    VServer               *_vsrv;
    std::vector<VServer *> _potentialVsrvs;

    RequestHandler _reqHandler;

    Client(const Client& other);
    Client& operator=(const Client& other);
    Client();

    e_CliState _state;

  public:
    ~Client();

    Client(
        Webserv *wsrv, int fd, VServer *vsrv, const str& addr, in_port_t port);

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

    void setVsrvPort(u16 p);
    u16  getVsrvPort() const;

    bool isTimeout() const;
    bool isVirtual() const;

    bool isIdling() const;
    bool isReading() const;
    bool isSending() const;
    bool isDisco() const;
    bool isDraining() const;
    bool isCGIing() const;

    void                    setPotentialVsrvs(std::vector<VServer *> vv);
    std::vector<VServer *>& getPotentialVsrvs();

    static Client *newVirtualCli(Webserv *w, int listenFd);

    void handleEvent(u32 ev);
    void handleEventCGI(u32 ev);

    void addCgiToEpoll(int fdWrite, int fdRead);
    void delCgiFromEpoll(int fd);
};
