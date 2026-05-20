/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:35:29 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/20 11:59:45 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Epoll.hpp"
#include "VServer.hpp"
#include "WsrvLib.hpp"

#include <map>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <vector>

// NOTE: Naming convention for private class-members / methods: underscore at
// the beginning. In later use this enables us to omit the `this->`. This saves
// a lot of typing while being clear enough as we are still hand-writing our
// code here ;)

class Webserv {
  private:
    bool                 _defaultCfg;
    bool                 _shutdown_server;
    std::vector<VServer> _vservers;

    // this maps a server-socket fd to a list of vservers.
    std::map< int, std::vector<VServer *> > _vserverFdMap;

    // this maps the a client fd to a list of possible vservers. when the first
    // request is send it will be decided which server is responsible for
    // handling it by serverName
    std::map< int, Client * > _fdClientMap;

    u16 _numOfClients;

    Epoll _epoll;

    // for storing the global env
    char **_envp;

    void _setupServers();
    void _setupSingleServer(std::vector<VServer>::iterator srvIt);
    void _setupEpoll();
    void _shutdownAllServers();

    // utils
    bool _isServerFd(int fd) const;

    // QUESTION: maybe this can be const / or ref?
    std::vector<VServer *> _getServersByFd(int fd);

    void _initDefaultCfg();
    void _printSockname(int sock);

    // we do not use them, so keep em private
    Webserv(const Webserv& other);
    Webserv& operator=(const Webserv& other);
    Webserv();

    void _timeoutClients();

    void _handleDiscoCGI(Client *cli, VServer *vsrv);

  public:
    Webserv(char **envp);
    ~Webserv();

    void run();
    void readConfig(const str& cfgFilename);
    void shutdownWebserv();

    void addCgiCliToEpoll(Client *cli, int fdWrite, int fdRead);
    void removeCgiFdFromEpoll(int fd);

    bool isVsrvName(constr& s) const;

    class WebservInitException: public std::runtime_error {
      public:
        WebservInitException(const std::string& msg);
    };

    class WebservRunException: public std::runtime_error {
      public:
        WebservRunException(const std::string& msg);
    };
};
