/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 23:39:57 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/04 16:00:21 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "VServer.hpp"
#include "WsrvLib.hpp"
#include "utils.hpp"

// --------------------------------=[ OCF ]=-------------------------------- //

Request::Request():
  _vsrv(NULL), _reqstr(""), _hdrLines(0), _reqFinished(false),
  _hdrComplete(false), _closeConn(false)
{}

Request::Request(const Request& o)
{
  if (this != &o) {
    _vsrv        = o._vsrv;
    _cli         = o._cli;
    _reqstr      = o._reqstr;
    _respo       = o._respo;
    _statusCode  = o._statusCode;
    _reqline     = o._reqline;
    _headers     = o._headers;
    _reqFinished = o._reqFinished;
    _hdrComplete = o._hdrComplete;
    _hdrLines    = o._hdrLines;
    _closeConn   = o._closeConn;
  }
}

Request& Request::operator=(const Request& o)
{
  if (this != &o) {
    _vsrv        = o._vsrv;
    _cli         = o._cli;
    _reqstr      = o._reqstr;
    _respo       = o._respo;
    _statusCode  = o._statusCode;
    _reqline     = o._reqline;
    _headers     = o._headers;
    _reqFinished = o._reqFinished;
    _hdrComplete = o._hdrComplete;
    _hdrLines    = o._hdrLines;
    _closeConn   = o._closeConn;
  }
  return *this;
}

Request::~Request() {}

// ------------------------------=[ END OCF ]=------------------------------ //

// the standard ctor we use for initializing a request in
// RequestHandler::read_request
Request::Request(Client *cli, const std::string& reqstr)
{
  _vsrv        = cli->getVsrv();
  _cli         = cli;
  _reqstr      = reqstr;
  _reqFinished = false;
  _hdrComplete = false;
  _hdrLines    = _countReqLines(reqstr);
  _closeConn   = false;
}

// When we hit this function, which is only done at the bottom of
// RquestHandler::read_request, the header will already be parsed and found to
// be ok. That's we here only have to deal with response-generation?!
void Request::_parseRequest()
{
  // FIXME: this is not valid procedure for requests with a body!
  if (!_isTerminatedReq())
    _statusCode = HTTP_400;
  else
    _statusCode = _respo.genResponse(*this);
}

// check if received request was terminated with '\r\n'
bool Request::_isTerminatedReq()
{
  if (_reqstr.size() < 4)
    return (false);
  if (_reqstr.compare(_reqstr.size() - 4, 4, "\r\n\r\n") == 0)
    return (true);
  return (false);
}

void Request::setFinished()
{
  _reqFinished = true;
  _parseRequest();
}

void Request::append(const str& s)
{
  _reqstr += s;
  _hdrLines += _countReqLines(s);
  Logger::log_reqres(_vsrv->getName(), "Appending to Req:", s);
}

// @brief Simply checks if `CRLFCRLF` is found somewhere in the reqstr. for
// obvious efficiency reasons starts from the back of _reqstr.
bool Request::hdrComplete()
{
  if (_hdrComplete)
    return true;
  _hdrComplete = _reqstr.rfind(CRLFX2) != str::npos;
  return _hdrComplete;
}

// for now we only look at the hdr
// TODO: we will also have to look for Content-Length!
bool Request::reqComplete()
{
  if (_hdrComplete) {
    return true;
  }
  return false;
}

e_Method Request::getMethod()
{
  if (hdrComplete())
    return _reqline.method;
  return M_UNKNOWN;
}

str Request::getMethodStr() const
{
  switch (_reqline.method) {
  case M_GET:
    return "GET";
  case M_POST:
    return "POST";
  case M_DELETE:
    return "DELETE";
  default:
    return "UNKNOWN";
  }
}

// FIXME: make sure this is bullet-proof and nothing is missed-out!
void Request::reset()
{
  _reqstr.clear();
  _statusCode          = HTTP_200;
  _hdrLines            = 0;
  _hdrComplete         = false;
  _reqFinished         = false;
  _reqline.httpVersion = HTTPVER_UNKNOWN;
  _reqline.target.clear();
  _reqline.method = M_GET;
  _headers.clear();
  _respo.reset();
}

e_HTTPStatus Request::parseHeaders()
{
  if ((_statusCode = parseReqLine()) != HTTP_200) {
    Logger::log_dbg1("Request::parseReqline: >=400");
    return _statusCode;
  }
  if ((_statusCode = _parseHeaders()) != HTTP_200) {
    Logger::log_dbg1("Request::_parseHeaders: >=400");
    return _statusCode;
  }
  if ((_statusCode = checkHeaders()) != HTTP_200) {
    Logger::log_dbg1("Request::checkHeaders: >=400");
    return _statusCode;
  }
  return HTTP_200;
}

// READ_BUFSIZE is set to 4096 bytes so half of this is in theory the max num of
// CRLFs in a string that can be checked -> u16 is enough.
// NOTE: empty lines are also counted
u16 Request::_countReqLines(const str& s)
{
  u16 lineNum = 0;

  size_t i     = 0;
  size_t ssize = s.size();

  while (i < ssize) {
    i = s.find(CRLF, i);
    if (i == str::npos)
      break;
    if ((i != str::npos && i + 3 < ssize && s.compare(i, 4, CRLFX2) == 0)) {
      lineNum += 2;
      break;
    }
    lineNum++;
    i += 2;
  }
  return lineNum;
}

void Request::setVsrv(VServer *v) { _vsrv = v; }

bool Request::hdrTooBig() const { return _hdrLines > MAX_HEADER_LINES; }

void Request::setStatusCode(e_HTTPStatus code) { _statusCode = code; }

bool Request::reqError() const { return _statusCode >= HTTP_400; }

// should connection be closed?
bool Request::closeConn() const { return _closeConn; }

const str& Request::getReqstr() const { return _reqstr; }

Client *Request::getCli() const { return _cli; }

VServer *Request::getVsrv() const { return _vsrv; }

e_HTTPStatus Request::getStatusCode() const { return _statusCode; }

const t_RequestLine& Request::getReqline() const { return _reqline; }

const std::map<str, str>& Request::getHeaders() const { return _headers; }

str Request::getResponseStr() const { return _respo.getRespoStr(); }

bool Request::isFinished() const { return _reqFinished; }
