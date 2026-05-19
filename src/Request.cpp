/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 23:39:57 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/19 15:10:59 by fmaurer          ###   ########.fr       */
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

Request& Request::operator=(const Request& o)
{
  if (this != &o) {
    _vsrvName      = o._vsrvName;
    _cli           = o._cli;
    _reqdata       = o._reqdata;
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
    _body          = o._body;
    _reqlineParsed = o._reqlineParsed;
    _hdrsParsed    = o._hdrsParsed;
    _contentLength = o._contentLength;
    _bodySize      = o._bodySize;
    _target        = o._target;
    _hostPort      = o._hostPort;
    _host          = o._host;
  }
  return *this;
}

Request::Request(const Request& o) { *this = o; }

Request::~Request() {}

// ------------------------------=[ END OCF ]=------------------------------ //

// the standard ctor we use for initializing a request in
// RequestHandler::read_request
Request::Request(Client *cli, const char *reqstr, size_t reqstrLen)
{
  _cli = cli;

  // safe-setting the _vsrvName
  if (_cli->isVirtual())
    _vsrvName = "__VIRTUAL__";
  else
    _vsrvName =
        (_cli->getVsrv() != NULL ? _cli->getVsrv()->getName() : "__VIRTUAL__");

  _statusCode          = HTTP_200;
  _hdrLines            = _countReqLines(reqstr);
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
  _reqlineParsed       = false;
  _hdrsParsed          = false;
  _contentLength       = 0;
  _closeConn           = false;
  _requestTarget       = "";
  _bodySize            = 0;
  _hostPort            = _cli->getVsrvPort();

  _reqdata.assign(reqstr, reqstrLen);
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

  // filter out virtual clients immediately!
  if (_cli->getVsrv() == NULL) {
    _statusCode = _respo.generateResponse(*this);
    return;
  }

  // should already be done until here, but better safe then deref NULL
  if (!this->badRequest() && _matchedRoute == NULL)
    this->evaluateTarget();

  // badRequest() is true for status >= 400. so, for statuses like 500 or 413 it
  // still would be nice to display the custom statusPage, if any. Therefore ->
  // match route.
  if (_statusCode != HTTP_400 && _matchedRoute == NULL)
    _matchRoute();

  // for CGI requests that are already bad don't do anything CGI related
  if (this->isCGI() && !this->badRequest()) {
    _statusCode = _respo.handleCGI(*this);
    if (this->badRequest())
      _statusCode = _respo.generateResponse(*this);
  }
  else
    _statusCode = _respo.generateResponse(*this);
}

// Match the route from req. After this there will _ALWAYS_ be a route set, if
// we don't throw exception.
//
// NOTE std::map stores items weakly ordered by there keys. that is the
// std::map::begin() iterator will always point to the smallest element if keys
// are int. coneversely, end() - 1 will be the largest.
//
void Request::_matchRoute()
{
  if (_cli->getVsrv() == NULL)
    throw std::runtime_error(
        "(Request::_matchRoute) _vsrv == NULL, client still virtual!");

  std::map<str, Route>& vsrvRoutes = _cli->getVsrv()->getRoutes();

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

// also handle the ability to "drain" the socket if the request was already
// terminated prematurely due to too large content.
void Request::append(char *s, ssize_t bytesRead)
{
  if (_cli->isDraining()) {
    _bodySize += bytesRead;
    Logger::logBug("draining...");
    if (_bodySize >= _contentLength) {
      _cli->setState(CLI_SEND);
      Logger::logBug("FINISHED DRAINING!");
    }
    return;
  }
  else if (_hdrComplete) {
    Logger::logDbg2("Appending this to body now:");
    Logger::logDbg2(data2hexStr(s, bytesRead));
    _bodySize += bytesRead;
    switch (_body.appendData(s, bytesRead)) {
      case 1:
        break;
      case 0:
        Logger::logSrv(
            _vsrvName, "(RequestBody::appendData) truncating body data!", WARN);
        break;
      default:
        Logger::logErr(
            "RequestBody::appendData", "Could not append to bodyData!");
    }
  }
  else {
    _reqdata.append(s, bytesRead);
    _hdrLines += _countReqLines(s);
  }
  Logger::logDbg1("Request::append",
      "Appending to req: '" + data2hexStr(s, bytesRead) + "'");
}

// @brief Simply checks if `CRLFCRLF` is found somewhere in the reqstr. for
// obvious efficiency reasons starts from the back of _reqstr.
bool Request::hdrComplete()
{
  if (_hdrComplete)
    return true;
  _hdrComplete = (_reqdata.rfind(CRLFX2) != str::npos);
  return _hdrComplete;
}

// check if a req is complete. for reqs != POST this is the case if the hdr is
// complete (we set _bodyComplete in _evaluateHeaders if req is not POST). for
// POST reqs this is the case if Content-Length bytes were read.
bool Request::reqComplete()
{
  // req != POST
  if (_hdrComplete && _bodyComplete)
    return true;

  Logger::logDbg1("Request::reqComplete",
      "bodysiz: " + int2str(_body.getSize()) +
          ", Content-Length: " + int2str(_contentLength));

  // req == POST. special treatment for draining situation necessary.
  if (_hdrComplete && (_bodySize >= _contentLength && !_cli->isDraining())) {
    Logger::logDbg1("Request::reqComplete", "Content-Length reached");
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

void Request::reset()
{
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
  _reqlineParsed       = false;
  _hdrsParsed          = false;
  _contentLength       = 0;
  _closeConn           = false;
  _bodySize            = 0;
  _hostPort            = 0;

  _reqdata.clear();
  _body.reset();
  _targetPath.clear();
  _requestTarget.clear();
  _reqline.target.clear();
  _headers.clear();
  _respo.reset();
  _target.clear();
  _host.clear();
}

// READ_BUFSIZE is set to 4096 bytes so half of this is in theory the max num
// of CRLFs in a string that can be checked -> u16 is enough. NOTE: empty
// lines are also counted
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

// --------------------=[ one-liners, more or less... ]=-------------------- //

void Request::setVsrvName(constr& name) { _vsrvName = name; }

bool Request::hdrTooBig() const { return _hdrLines > MAX_HEADER_LINES; }

void Request::setStatusCode(e_HTTPStatus code)
{
  _statusCode = code;
  _respo.setStatus(code);
}

bool Request::reqError() const { return _statusCode >= HTTP_400; }

// should connection be closed?
bool Request::closeConn() const { return _closeConn; }

const str& Request::getReqstr() const { return _reqdata; }

Client *Request::getCli() const { return _cli; }

str Request::getVsrvName() const { return _vsrvName; }

e_HTTPStatus Request::getStatus() const { return _statusCode; }

t_RequestLine& Request::getReqline() { return _reqline; }

std::map<str, str>& Request::getHeaders() { return _headers; }

str Request::getResponseStr() const { return _respo.getRespoStr(); }

Route *Request::getMatchedRoute() { return _matchedRoute; }

constr& Request::getTargetPath() const { return _targetPath; }

// return true if client is not in draining state and statusCode is somewhere in
// the error range. Draining state can only be released in Request::append iff
// we finally surpass the contentLength.
bool Request::badRequest() const
{
  return (!_cli->isDraining() && _statusCode >= HTTP_400);
}

bool Request::isCGI() const { return _isCGI; }

bool Request::isSimplePOST() const { return _isSimplePOST; }

bool Request::isDELETE() const { return _isDELETE; }

bool Request::isRedir() const { return _isRedir; }

const std::pair<e_HTTPStatus, URI>& Request::getRedir() const { return _redir; }

// if we have received at least 2 lines or the maximum reqline length was
// already received we should check at least if the reqline is correct
bool Request::reqlineReceived() const
{
  if (_reqlineParsed)
    return _reqlineParsed;
  return (_hdrLines >= 2 || _reqdata.size() >= MAX_REQLINE_LEN);
}
bool Request::reqlineParsed() const { return _reqlineParsed; }

RequestBody& Request::getBody() { return _body; }

std::vector<char>& Request::getBodyData() { return _body.getBodyData(); }

bool Request::hdrsParsed() const { return _hdrsParsed; }

size_t Request::getBodySize() const { return _bodySize; }

void Request::setBodySize(size_t s) { _bodySize = s; }

void Request::setHost(str host) { _host = host; }

str Request::getHost() const { return _host; }

void Request::setHostPort(u16 p) { _hostPort = p; }

u16 Request::getHostPort() const { return _hostPort; }

Response& Request::getRespo() { return _respo; }

void Request::setCGIdone() { _isCGI = false; }

// CGI wrappers

void Request::cgiWrite() { _respo.cgiWrite(); }
bool Request::cgiEvalChildState() { return _respo.cgiEvalChildState(); }
void Request::cgiRead() { _respo.cgiRead(); }
void Request::cgiProcessBody() { _respo.cgiProcessBody(); }
void Request::cgiCleanupFds() { _respo.cgiCleanupFds(); }
void Request::cgiKillProcess() { _respo.cgiKillProcess(); }
bool Request::cgiIsWriteFd(int fd) const { return _respo.cgiIsWriteFd(fd); }
bool Request::cgiIsReadFd(int fd) const { return _respo.cgiIsRead(fd); }
