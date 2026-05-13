/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 23:39:57 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/13 19:23:07 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "VServer.hpp"
#include "WsrvLib.hpp"
#include "utils.hpp"

// --------------------------------=[ OCF ]=-------------------------------- //

Request::Request() { this->reset(); }

Request::Request(const Request& o)
{
  if (this != &o) {
    _vsrv          = o._vsrv;
    _cli           = o._cli;
    _reqstr        = o._reqstr;
    _respo         = o._respo;
    _statusCode    = o._statusCode;
    _reqline       = o._reqline;
    _headers       = o._headers;
    _hdrComplete   = o._hdrComplete;
    _bodyComplete  = o._bodyComplete;
    _hdrLines      = o._hdrLines;
    _closeConn     = o._closeConn;
    _requestTarget = o._requestTarget;
    _targetPath    = o._targetPath;
    _matchedRoute  = o._matchedRoute;
    _isCGI         = o._isCGI;
    _isSimplePOST  = o._isSimplePOST;
    _isDELETE      = o._isDELETE;
    _isRedir       = o._isRedir;
    _redir         = o._redir;
  }
}

Request& Request::operator=(const Request& o)
{
  if (this != &o) {
    _vsrv          = o._vsrv;
    _cli           = o._cli;
    _reqstr        = o._reqstr;
    _respo         = o._respo;
    _statusCode    = o._statusCode;
    _reqline       = o._reqline;
    _headers       = o._headers;
    _hdrComplete   = o._hdrComplete;
    _bodyComplete  = o._bodyComplete;
    _hdrLines      = o._hdrLines;
    _closeConn     = o._closeConn;
    _requestTarget = o._requestTarget;
    _targetPath    = o._targetPath;
    _matchedRoute  = o._matchedRoute;
    _isCGI         = o._isCGI;
    _isSimplePOST  = o._isSimplePOST;
    _isDELETE      = o._isDELETE;
    _isRedir       = o._isRedir;
    _redir         = o._redir;
  }
  return *this;
}

Request::~Request() {}

// ------------------------------=[ END OCF ]=------------------------------ //

// the standard ctor we use for initializing a request in
// RequestHandler::read_request
Request::Request(Client *cli, const std::string& reqstr)
{
  _vsrv          = cli->getVsrv();
  _cli           = cli;
  _reqstr        = reqstr;
  _hdrComplete   = false;
  _bodyComplete  = false;
  _hdrLines      = _countReqLines(reqstr);
  _closeConn     = false;
  _requestTarget = "";
  _matchedRoute  = NULL;
  _isCGI         = false;
  _isSimplePOST  = false;
  _isDELETE      = false;
  _isRedir       = false;
  _redir         = std::make_pair(HTTP_0, "");
}

// There are 2 options when we get here:
//
//  1) Request was valid and so we do normal processing.
//
//  2) Request was bad, the only thing we have to do is properly look up the
//     right error page from server scope and feed that into the respostr. If
//     that fails we have to return default 404
void Request::processReq()
{

  // iff request wasn't bad try to match route
  if (this->badRequest() == false) {
    _matchRoute();

    Logger::logBug("MATCHED ROUTE: " + _matchedRoute->getPath());
    Logger::logBug("TARGET PATH: " + _targetPath);

    // classify request a little
    _isCGI        = !_matchedRoute->getCgi().empty();
    _isSimplePOST = (!_isCGI && _reqline.method == M_POST);
    _isDELETE     = (_reqline.method == M_DELETE);

    _isRedir = (_matchedRoute->getRedir().first != HTTP_0);
    if (_isRedir)
      _redir = _matchedRoute->getRedir();
  }

  _statusCode = _respo.generateResponse(*this);
}

// QUESTION: is the targetPath validated enough when we come here?
//
// std::map stores items weakly ordered by there keys. that is the
// std::map::begin() iterator will always point to the smallest element if keys
// are int. coneversely, end() - 1 will be the largest.
//
//
// how to match target == `/miep/index.html` with `/miep`?
void Request::_matchRoute()
{
  std::map<str, Route>& vsrvRoutes = _vsrv->getRoutes();

  // return direct matches immediately
  if (vsrvRoutes.find(_requestTarget) != vsrvRoutes.end()) {
    _matchedRoute = &vsrvRoutes[_requestTarget];
    return;
  }

  std::map<size_t, Route *> rank;

  for (std::map<str, Route>::iterator rit = vsrvRoutes.begin();
      rit != vsrvRoutes.end();
      ++rit)
  {
    Route& r         = rit->second;
    str    routePath = r.getPath();
    size_t i         = 0;
    while (i < routePath.size() && i < _requestTarget.size() &&
        routePath[i] == _requestTarget[i])
      ++i;

    // this will always be at least true for the default route '/'
    if (i == routePath.size())
      rank.insert(std::make_pair(i, &r));
  }

  if (rank.size() > 0)
    _matchedRoute = (--rank.end())->second;
  else
    _matchedRoute = &(vsrvRoutes.begin()->second); // return default route

  size_t ps = _matchedRoute->getPath().size();
  if (_requestTarget.size() > ps)
    _targetPath = _requestTarget.substr(ps);
}

// TODO: add body separation here for POST reqs
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
  _hdrComplete = (_reqstr.rfind(CRLFX2) != str::npos);
  return _hdrComplete;
}

// for now we only look at the hdr
// TODO: we will also have to look for Content-Length!
bool Request::reqComplete() { return (_hdrComplete && _bodyComplete); }

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
  _bodyComplete        = false;
  _reqline.httpVersion = HTTPVER_UNKNOWN;
  _reqline.method      = M_GET;
  _matchedRoute        = NULL;
  _isCGI               = false;
  _isSimplePOST        = false;
  _isDELETE            = false;
  _isRedir             = false;
  _redir               = std::make_pair(HTTP_0, "");

  _targetPath.clear();
  _requestTarget.clear();
  _reqline.target.clear();
  _headers.clear();
  _respo.reset();
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

std::map<str, str>& Request::getHeaders() { return _headers; }

str Request::getResponseStr() const { return _respo.getRespoStr(); }

Route *Request::getMatchedRoute() { return _matchedRoute; }

constr& Request::getTargetPath() const { return _targetPath; }

// iff at any point we end up with a HTTP_400 the header was bad in some way.
bool Request::badRequest() const { return _statusCode == HTTP_400; }

bool Request::isCGI() const { return _isCGI; }

bool Request::isSimplePOST() const { return _isSimplePOST; }

bool Request::isDELETE() const { return _isDELETE; }

bool Request::isRedir() const { return _isRedir; }

const std::pair<e_HTTPStatus, str>& Request::getRedir() const { return _redir; }
