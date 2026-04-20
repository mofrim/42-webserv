/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VServerCfg.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 08:01:29 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/20 23:16:37 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Route.hpp"
#include "typesAndConstants.hpp"

#include <map>
#include <netinet/in.h>
#include <set>
#include <string>
#include <sys/socket.h>
#include <vector>

class VServerCfg {
  private:
    std::string                   _server_name;
    std::map<str, std::set<u16> > _interfaces;
    std::vector<Route>            _routes;

  public:
    // OCF
    VServerCfg();
    VServerCfg(const VServerCfg& other);
    VServerCfg& operator=(const VServerCfg& other);
    ~VServerCfg();

    void        setServerName(std::string name);
    std::string getServerName() const;

    void addInterface(const str& addr, u16 port);
    void addInterfaces(std::map<str, std::set<u16> > _interfaces);
    const std::map< str, std::set<u16> >& getInterfaces() const;
    const std::set<u16>&                  getPorts(const str& addr);
    int delPort(const str& interface, u16 port);
    int delInterface(const str& interface);

    void printCfg() const;
};
