/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 23:39:07 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/23 10:16:23 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"
#include "typesAndConstants.hpp"

#include <string>

// The Request class.
//
// - at the end of parsing _statusCode will be set. if, f.ex., _statusCode in
// [400 - 499] we need to display one of the error pages
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
    u16  _statusCode;
    bool _reqFinished;

    t_RequestLine _reqline;

    bool _isTerminatedReq();
    void _parseRequest();

  public:
    // TODO: decide which ctors we really use here!
    Request();
    Request(const Request& other);
    Request(VServer *srv, Client *cli, const std::string& rstr);
    Request& operator=(const Request& other);
    ~Request();

    std::string getResponse() const;

    bool isFinished() const;
    void setFinished();

    void append(const str& s);

    bool     hdrComplete() const;
    bool     reqComplete();
    e_Method getMethod() const;
};
