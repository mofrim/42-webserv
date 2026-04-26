/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 23:39:57 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/26 17:58:39 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Logger.hpp"
#include "ReqParse.hpp"
#include "Request.hpp"
#include "WsrvLib.hpp"
#include "utils.hpp"

// --------------------------------=[ OCF ]=-------------------------------- //

Request::Request(): _vsrv(NULL), _reqstr(""), _reqFinished(false)
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
  }
  return (*this);
}

Request::~Request()
{}

// ------------------------------=[ END OCF ]=------------------------------ //

// the standard ctor we use for initializing a request _and_ parse the request
// at the same time.
Request::Request(Client *cli, const std::string& reqstr)
{
  _vsrv        = cli->getVsrv();
  _cli         = cli;
  _reqstr      = reqstr;
  _reqFinished = false;
}

// Important resource:
//
// 	https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Status

void Request::_parseRequest()
{
  if (!_isTerminatedReq())
    _statusCode = HTTP_400;
  else
    _statusCode = ReqParse::parseReqLine(_reqline, _reqstr);
  _statusCode = _respo.genResponse(*this);
}

str Request::getResponseStr() const
{
  return _respo.getRespoStr();
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

bool Request::isFinished() const
{
  return _reqFinished;
}

void Request::setFinished()
{
  _reqFinished = true;
  _parseRequest();
}

void Request::append(const str& s)
{
  _reqstr += s;
  Logger::log_reqres("Appending to req:", s);
}

bool Request::hdrComplete() const
{
  if (_reqstr.rfind("\r\n\r\n") != str::npos)
    return true;
  return false;
}

// for now we only look at the hdr
bool Request::reqComplete() const
{
  if (hdrComplete()) {
    Logger::log_msg("Request::reqComplete: Request complete!");
    return true;
  }
  return false;
}

e_Method Request::getMethod() const
{
  if (hdrComplete())
    return _reqline.method;
  return M_UNKNOWN;
}

const str& Request::getReqstr() const
{
  return _reqstr;
}

Client *Request::getCli() const
{
  return _cli;
}

VServer *Request::getVsrv() const
{
  return _vsrv;
}

u16 Request::getStatusCode() const
{
  return _statusCode;
}

const t_RequestLine& Request::getReqline() const
{
  return _reqline;
}

const std::map<str, str>& Request::getHeaders() const
{
  return _headers;
}

void Request::reset()
{
  _reqstr.clear();
  _statusCode  = 0;
  _reqFinished = false;
  _reqline.httpVersion.clear();
  _reqline.target.clear();
  _reqline.method = M_GET;
  _headers.clear();
  _respo.reset();
}
