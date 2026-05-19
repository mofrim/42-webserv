/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServerCfg.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 08:01:29 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/19 13:04:55 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Route.hpp"
#include "WsrvLib.hpp"

#include <map>
#include <netinet/in.h>
#include <set>
#include <string>
#include <sys/socket.h>

class VServerCfg {
  private:
    std::string                   _serverName;
    std::map<str, std::set<u16> > _interfaces;
    u32                           _maxBodySize;
    str                           _root;

    std::map<e_HTTPStatus, str> _errPages;

    // keys for _routes should be the _path prop of a Route.
    std::map<str, Route> _routes;

    // FIXME: remove!!!! DEAD CODE!!!!
    // to keep an overview which directives where set from cfg file.
    std::map<str, bool> _setDirecs;
    void                _initSetDirecs();

  public:
    VServerCfg();
    VServerCfg(const VServerCfg& other);
    VServerCfg& operator=(const VServerCfg& other);
    ~VServerCfg();

    void printCfg() const;

    void        setServerName(std::string name);
    std::string getServerName() const;

    void addInterface(const str& addr, u16 port);
    void addInterfaces(std::map<str, std::set<u16> > _interfaces);

    const std::map< str, std::set<u16> >& getInterfaces() const;
    const std::set<u16>&                  getPorts(const str& addr);

    int delPort(const str& interface, u16 port);
    int delInterface(const str& interface);

    void                        setRoutes(const std::map<str, Route>& routes);
    const std::map<str, Route>& getRoutes() const;
    void                        addRoute(const Route& r);

    void setMaxBodySize(u32 mbs);
    u32  getMaxBodySize() const;

    void setRoot(str r);
    str  getRoot() const;

    void                        addErrPage(e_HTTPStatus s, const str& path);
    str                         getErrPage(e_HTTPStatus s);
    std::map<e_HTTPStatus, str> getErrPages() const;

    bool checkEnsureCfg();
};
