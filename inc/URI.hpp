/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   URI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/06 22:32:20 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/20 11:28:32 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <stdint.h>
#include <string>

// setting a bit of arbitrary max URL-authority length here. but i realy
// don't want to deal with all details of URL parsing!
#define URL_MAX_AUTH_LENGTH 2084

// forward typedef as WsrvLib.hpp can not be included here
typedef std::string       str;
typedef const std::string constr;
typedef uint16_t          u16;

// URL class
//
// There will be 2 canonical ways of instantiating an URL:
//
//  1) via defaullt ctor and URL::parseURL
//  2) via URL(const str& target) ctor
//
// In both cases URL::bad() should be checked.
//
// Main resource: https://datatracker.ietf.org/doc/html/rfc3986
class URI {
  private:
    str                _scheme;
    str                _auth;
    str                _host;
    u16                _port;
    str                _path;
    std::map<str, str> _query;
    str                _fragment;
    bool               _bad;
    bool               _empty;

  public:
    URI();
    URI(const URI& other);
    URI& operator=(const URI& other);
    ~URI();
    URI(const str& u);

    str parsePath(constr& p);
    str parseURL(constr& u);

    bool                bad() const;
    bool                empty() const;
    str                 getPath() const;
    std::map<str, str>& getQuery();
    str                 getQueryCSStr() const;
    str                 getQueryStr() const;
    str                 getStr() const;
    void                clear();
    bool                isURL() const;
    u16                 getPort() const;
    str                 getHost() const;
    str                 getAuth() const;
};
