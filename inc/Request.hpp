/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 23:39:07 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/14 07:56:45 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// TODO:
//
// - implement timeout

#include "Response.hpp"
#include "Route.hpp"
#include "WsrvLib.hpp"

#include <map>
#include <unistd.h>
#include <vector>

// The Request class.

class VServer;
class Client;

class Request {
  private:
    VServer *_vsrv;
    Client  *_cli;
    str      _reqdata;
    Response _respo;

    e_HTTPStatus _statusCode;
    u16          _hdrLines;
    bool         _hdrComplete;
    bool         _bodyComplete;
    bool         _closeConn;

    // doing it exactly as proposed in:
    // https://datatracker.ietf.org/doc/html/rfc9112#section-2.2
    t_RequestLine      _reqline;
    std::map<str, str> _headers;

    str    _requestTarget; // the targetstr from requestline
    Route *_matchedRoute;
    str    _targetPath; // the target-path minus the route

    std::pair<e_HTTPStatus, str> _redir;

    bool _isCGI;        // CGI requests can be both POST and GET
    bool _isSimplePOST; // simple POST without CGI
    bool _isDELETE;     // DELETEs only have a filename as query param
    bool _isRedir;      // if route is a redir route

    u16 _countReqLines(const str& nstr);

    // Parsing related
    std::vector< std::pair<str, str> > _splitHdr();
    size_t                             _skipEmptyHdrLine() const;
    e_HTTPStatus                       _readReqline();
    e_HTTPStatus                       _parseHeaders();
    void                               _matchRoute();

    e_HTTPStatus _evaluateHdrs();

  public:
    // TODO: decide which ctors we really use here & privatize unused
    Request();
    Request(const Request& other);
    Request(Client *cli, const str& rstr);
    Request& operator=(const Request& other);
    ~Request();

    str getResponseStr() const;

    void processReq();

    void append(char *s, ssize_t readBytes);

    e_Method     getMethod();
    constr&      getReqstr() const;
    Client      *getCli() const;
    VServer     *getVsrv() const;
    void         setVsrv(VServer *v);
    e_HTTPStatus getStatusCode() const;
    void         setStatusCode(e_HTTPStatus code);
    str          getMethodStr() const;
    bool         closeConn() const;

    const std::pair<e_HTTPStatus, str>& getRedir() const;

    bool reqError() const;

    void reset();

    // FIXME: maybe refs are okay here
    const t_RequestLine& getReqline() const;
    std::map<str, str>&  getHeaders();
    void                 evaluateTarget();

    // parsing
    // FIXME: which function should i expose here?

    e_HTTPStatus parseReqHeaders();
    e_HTTPStatus parseReqLine();

    Route  *getMatchedRoute();
    constr& getTargetPath() const;

    bool badRequest() const;
    bool hdrComplete();
    bool reqComplete();
    bool hdrTooBig() const;

    bool isCGI() const;
    bool isSimplePOST() const;
    bool isDELETE() const;
    bool isRedir() const;

    bool reqlineReceived() const;
};
