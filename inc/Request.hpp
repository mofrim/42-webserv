/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 23:39:07 by fmaurer           #+#    #+#             */
/*   Updated: 2026/06/13 11:01:54 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "RequestBody.hpp"
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
    Client     *_cli;
    str         _reqdata;
    Response    _respo;
    RequestBody _body;

    e_HTTPStatus _statusCode;
    u16          _hdrLines;
    bool         _hdrComplete;
    bool         _bodyComplete;
    bool         _reqlineParsed;
    bool         _hdrsParsed;
    bool         _closeConn;
    size_t       _contentLength;
    str          _vsrvName;
    size_t       _bodySize;
    bool         _isChunked;

    str _host;
    u16 _hostPort;

    // doing it exactly as proposed in:
    // https://datatracker.ietf.org/doc/html/rfc9112#section-2.2
    t_RequestLine      _reqline;
    std::map<str, str> _headers;

    str    _requestTarget; // the targetstr from requestline
    Route *_matchedRoute;
    str    _targetPath; // the target-path minus the route
    URI    _target;

    std::pair<e_HTTPStatus, URI> _redir;

    bool _isCGI;        // CGI requests can be both POST and GET
    bool _isSimplePOST; // simple POST without CGI
    bool _isDELETE;     // DELETEs only have a filename as query param
    bool _isRedir;      // if route is a redir route
    bool _isPOST;       // POST

    u16 _countReqLines(const str& nstr);
    str _getReqlineAsStr() const;

    // Parsing related
    std::vector< std::pair<str, str> > _splitHdr();
    size_t                             _skipEmptyHdrLine() const;
    e_HTTPStatus                       _readReqline();
    e_HTTPStatus                       _parseHeaders();
    void                               _matchRoute();
    void                               _logSelectedHdrs();

    e_HTTPStatus _evaluateHdrs();
    e_HTTPStatus _initializeBody(constr& onlyHdrs, size_t crlfx2);

  public:
    Request();
    Request(const Request& other);
    Request(Client *cli, const char *reqstr, size_t reqstrLen);
    Request& operator=(const Request& other);
    ~Request();

    const str& getResponseStr() const;

    void processReq();

    void append(char *s, ssize_t readBytes);

    e_Method     getMethod();
    str          getReqstr() const;
    Client      *getCli() const;
    str          getVsrvName() const;
    void         setVsrvName(constr& name);
    e_HTTPStatus getStatus() const;
    void         setStatusCode(e_HTTPStatus code);
    str          getMethodStr() const;
    bool         closeConn() const;
    void         setCloseConn();

    const std::pair<e_HTTPStatus, URI>& getRedir() const;

    bool reqError() const;

    void reset();

    t_RequestLine&      getReqline();
    std::map<str, str>& getHeaders();
    void                evaluateTarget();

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
    void setCGIdone();

    bool reqlineReceived() const;
    bool reqlineParsed() const;
    bool hdrsParsed() const;

    RequestBody& getBody();
    const char  *getBodyRawData(size_t idx = 0) const;
    str          getBodyDataAsStr(size_t trunc = str::npos) const;
    size_t       getBodySize() const;
    void         setBodySize(size_t s);
    str          getHdrsAsStr() const;
    void         resetBody();

    void setHost(str host);
    str  getHost() const;

    void setHostPort(u16 p);
    u16  getHostPort() const;

    Response& getRespo();
    void      setStatusFromRespo();
    void      buildResponse();

    // CGI wrappers
    void  cgiWrite();
    int   cgiEvalChildState();
    void  cgiRead();
    void  cgiProcessBody();
    void  cgiCleanupFds();
    bool  cgiIsWriteFd(int fd) const;
    bool  cgiIsReadFd(int fd) const;
    pid_t cgiGetCpid() const;
};
