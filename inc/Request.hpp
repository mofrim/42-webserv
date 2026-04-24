/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 23:39:07 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/24 17:32:37 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// TODO:
//
// - implement timeout

#include "Client.hpp"
#include "Response.hpp"
#include "WsrvLib.hpp"

#include <map>
#include <string>

// The Request class.
//
// NEXT: implement Response class... it should hold the response header, utility
// functions for setting date, server name, content-length, content-type (!!!!
// it could also be an image!?!?) _and_ the response itself! Also we need to
// pass the statusCode and somehow decide, taking the servers cfg into account,
// what error page to display.

class VServer;

class Request {
  private:
    VServer *_vsrv;
    Client  *_cli;

    str _reqstr;
    str _response; // FIXME: leave this as a string ?! No! Can also
                   // be a stream of bytes. Will be another class!

    Response _Response;

    u16  _statusCode;
    bool _reqFinished;

    // doing it exactly as proposed in:
    // https://datatracker.ietf.org/doc/html/rfc9112#section-2.2
    t_RequestLine      _reqline;
    std::map<str, str> _headers;

    bool _isTerminatedReq();
    void _parseRequest();

  public:
    // TODO: decide which ctors we really use here & privatize unused
    Request();
    Request(const Request& other);
    Request(VServer *srv, Client *cli, const std::string& rstr);
    Request& operator=(const Request& other);
    ~Request();

    std::string getResponseStr() const;

    bool isFinished() const;
    void setFinished();

    void append(const str& s);

    bool hdrComplete() const;
    bool reqComplete();

    e_Method   getMethod() const;
    const str& getReqstr() const;
    Client    *getCli() const;
    VServer   *getVsrv() const;
    u16        getStatusCode() const;

    // FIXME: maybe refs are okay here
    const t_RequestLine&      getReqline() const;
    const std::map<str, str>& getHeaders() const;
};
