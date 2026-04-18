/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServer.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:50:36 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/18 20:18:30 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"
#include "RequestHandler.hpp"
#include "VServerCfg.hpp"
#include "typesAndConstants.hpp"

#include <map>
#include <set>
#include <stdexcept>

class VServer {
  private:
    // this will be multiple ports
    uint16_t      _port;
    std::set<u16> _ports;

    in_addr_t   _host;
    std::string _server_name;
    std::string _root;

    // this will also be an array
    struct sockaddr_in _server_addr;

    // this will be a list/set/vector
    int           _listen_fd;
    std::set<int> _listen_fds;

    std::map< str, std::set<u16> > _activeInterfaces;

    bool _setupFailed;

    // FIXME: maybe only keep the cfg in here and don't store ^^ those extra
    // values seperately bc this is redundant!
    VServerCfg _cfg;

    std::map<int, Client *> _clients;

    RequestHandler _reqHandler;

    void _setupSockets();
    void _removeAllClients();

  public:
    // OCF
    VServer();
    VServer(const VServerCfg& srvcfg);
    VServer(const VServer& other);
    VServer& operator=(const VServer& other);
    ~VServer();

    void    init();
    Client *addClient(int fd);
    void    removeClient(int fd);
    int     handleEvent(const struct epoll_event& ev, int client_fd);
    void    removeAllClients();

    // utils, getters setters
    in_addr_t         getHost() const;
    std::string       getServerName() const;
    std::string       getRoot() const;
    sockaddr_in       getServerAddr() const;
    const VServerCfg *getCfg() const;
    bool              getSetupFailed() const;
    u16               getNumOfListenFds() const;

    void setHost(in_addr_t host);
    void setServerName(std::string name);
    void setRoot(std::string root);
    void setListenFd(int listen_fd);
    void setServerAddr(sockaddr_in server_addr);
    void setSetupFailed();

    void printCfg() const;
    void printClients();
    bool isValidClientFd(int fd);

    // WIP:
    uint16_t getPort() const;
    void     setPort(uint16_t port);

    int                  getListenFd() const;
    const std::set<int>& getListenFds() const;
    const std::set<u16>& getPorts() const;

    void cleanup();

    // TODO: think about the whole exception thing! Maybe there should be
    // some base-class called like this one and the special exceptions for every
    // thing that can fail (~ sounds like a lot of work to me :/ )
    class ServerInitException: public std::runtime_error {
      public:
        ServerInitException(const std::string& msg);
    };

    class ServerException: public std::runtime_error {
      public:
        ServerException(const std::string& msg);
    };
};
