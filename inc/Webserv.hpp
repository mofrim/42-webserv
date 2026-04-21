/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:35:29 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/21 16:41:07 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Epoll.hpp"
#include "VServer.hpp"
#include "typesAndConstants.hpp"

#include <map>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

// NOTE: Naming convention for private class-members / methods: underscore at
// the beginning. In later use this enables us to omit the `this->`. This saves
// a lot of typing while being clear enough as we are still hand-writing our
// code here ;)

class Webserv {
  private:
    bool                     _defaultCfg;
    bool                     _shutdown_server;
    std::vector<VServer>     _servers;
    std::map<int, VServer *> _serverFdMap;
    std::map<int, VServer *> _clientFdServerMap;
    size_t                   _numOfServers;
    uint16_t                 _numOfClients;

    Epoll _epoll;

    void _setupServers();
    void _setupSingleServer(VServer& srv);
    void _setupEpoll();
    void _shutdownAllServers();

    // utils
    bool     _isServerFd(int fd) const;
    VServer *_getServerByFd(int fd);
    VServer *_getServerByClientFd(int fd);
    void     _addClientToClientFdServerMap(int fd, VServer *srv);
    void     _initDefaultCfg();
    void     _printSockname(int sock);

    // we do not use them, so keep em private
    Webserv(const Webserv& other);
    Webserv& operator=(const Webserv& other);

  public:
    Webserv();
    ~Webserv();

    void run();
    void getServersFromCfg(const std::string& cfgFilename);
    void shutdownWebserv();

    class WebservInitException: public std::runtime_error {
      public:
        WebservInitException(const std::string& msg);
    };

    class WebservRunException: public std::runtime_error {
      public:
        WebservRunException(const std::string& msg);
    };
};
