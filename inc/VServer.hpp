/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServer.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:50:36 by fmaurer           #+#    #+#             */
/*  Updated: 2026/04/20 20:40:40 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"
#include "RequestHandler.hpp"
#include "VServerCfg.hpp"
#include "WsrvLib.hpp"

#include <map>
#include <set>
#include <stdexcept>

class VServer {
  private:
    std::set<u16> _ports;
    std::string   _srvName;
    std::set<int> _listen_fds;
    std::set<int> _virtualFds;

    u32 _maxBodySize;

    std::map< str, std::set<u16> > _activeInterfaces;
    std::map< str, std::set<u16> > _activeAddrPortPairs;

    std::map< int, std::pair<str, u16> > _fdIfaceMap;

    std::map<str, Route> _routes;

    bool _setupFailed;

    std::map<int, Client *> _clients;

    void _setupSockets(std::vector<VServer>::iterator begin,
        std::vector<VServer>::iterator                cur);

    void _removeAllClients();

    int _findVirtualBuddy(std::vector<VServer>::iterator begin,
        std::vector<VServer>::iterator                   cur,
        const str&                                       addr,
        u16                                              port);

    int _isActiveIface(const str& addr, u16 port) const;

  public:
    // OCF
    VServer();
    VServer(const VServerCfg& srvcfg);
    VServer(const VServer& other);
    VServer& operator=(const VServer& other);
    ~VServer();

    void init(std::vector<VServer>::iterator begin,
        std::vector<VServer>::iterator       cur);

    Client *addClient(int fd);
    void    addClient(Client *cli);

    void deleteClient(int fd);

    // utils, getters setters
    std::string       getName() const;
    sockaddr_in       getServerAddr() const;
    const VServerCfg *getCfg() const;
    bool              getSetupFailed() const;
    u16               getNumOfListenFds() const;

    void setServerName(std::string name);
    void setSetupFailed();

    void printCfg() const;
    void printClients();
    bool isValidClientFd(int fd);
    bool isVirtualFd(int fd) const;

    const std::set<int>& getListenFds() const;
    const std::set<u16>& getPorts() const;

    void                        setRoutes(const std::map<str, Route>& r);
    const std::map<str, Route>& getRoutes() const;

    void setMaxBodySize(u32 mbs);
    u32  getMaxBodySize() const;

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
