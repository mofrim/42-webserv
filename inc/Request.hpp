/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 23:39:07 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/01 09:49:21 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// TODO:
//
// - implement timeout

#include "Response.hpp"
#include "WsrvLib.hpp"

#include <map>

// The Request class.

class VServer;
class Client;

class Request {
  private:
    VServer *_vsrv;
    Client  *_cli;
    str      _reqstr;
    Response _respo;

    e_HTTPStatus _statusCode;
    bool         _reqFinished;
    u16          _hdrLines;

    // doing it exactly as proposed in:
    // https://datatracker.ietf.org/doc/html/rfc9112#section-2.2
    t_RequestLine      _reqline;
    std::map<str, str> _headers;

    bool _isTerminatedReq();
    void _parseRequest();
    u16  _countReqLines(const str& nstr);

  public:
    // TODO: decide which ctors we really use here & privatize unused
    Request();
    Request(const Request& other);
    Request(Client *cli, const str& rstr);
    Request& operator=(const Request& other);
    ~Request();

    str getResponseStr() const;

    bool isFinished() const;
    void setFinished();

    void append(const str& s);

    bool hdrComplete() const;
    bool reqComplete() const;

    e_Method     getMethod() const;
    const str&   getReqstr() const;
    Client      *getCli() const;
    VServer     *getVsrv() const;
    void         setVsrv(VServer *v);
    e_HTTPStatus getStatusCode() const;
    void         setStatusCode(e_HTTPStatus code);
    bool         hdrTooBig() const;

    bool reqError() const;

    void reset();

    // FIXME: maybe refs are okay here
    const t_RequestLine&      getReqline() const;
    const std::map<str, str>& getHeaders() const;

    e_HTTPStatus parseHeaders();
};
