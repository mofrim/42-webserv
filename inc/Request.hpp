/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 23:39:07 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/04 10:57:30 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// TODO:
//
// - implement timeout

#include "Response.hpp"
#include "WsrvLib.hpp"

#include <map>
#include <vector>

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
    u16          _hdrLines;
    bool         _reqFinished;
    bool         _hdrComplete;
    bool         _closeConn;

    // doing it exactly as proposed in:
    // https://datatracker.ietf.org/doc/html/rfc9112#section-2.2
    t_RequestLine      _reqline;
    std::map<str, str> _headers;

    bool _isTerminatedReq();
    void _parseRequest();
    u16  _countReqLines(const str& nstr);

    // Parsing related
    std::vector< std::pair<str, str> > _splitHdr();
    e_HTTPStatus                       _readReqline();
    e_HTTPStatus                       _parseHeaders();

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

    bool hdrComplete();
    bool reqComplete();

    e_Method     getMethod();
    const str&   getReqstr() const;
    Client      *getCli() const;
    VServer     *getVsrv() const;
    void         setVsrv(VServer *v);
    e_HTTPStatus getStatusCode() const;
    void         setStatusCode(e_HTTPStatus code);
    bool         hdrTooBig() const;
    str          getMethodStr() const;
    bool         closeConn() const;

    bool reqError() const;

    void reset();

    // FIXME: maybe refs are okay here
    const t_RequestLine&      getReqline() const;
    const std::map<str, str>& getHeaders() const;

    // parsing
    // FIXME: which function should i expose here?

    e_HTTPStatus parseHeaders();
    e_HTTPStatus parseReqLine();
    e_HTTPStatus checkHeaders();
    int          validateUrl(const str& u);
};
