/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 19:11:25 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/14 08:17:15 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "VServer.hpp"
#include "WsrvLib.hpp"
#include "utils.hpp"

#include <fstream>

// --------------------------------=[ OCF ]=-------------------------------- //

Response::Response() { this->reset(); }

Response::Response(const Response& o)
{
  if (this != &o) {
    _status       = o._status;
    _reqline      = o._reqline;
    _cli          = o._cli;
    _vsrv         = o._vsrv;
    _respoHeaders = o._respoHeaders;
    _body         = o._body;
    _mimeType     = o._mimeType;
    _respoStr     = o._respoStr;
    _closeConn    = o._closeConn;
    _req          = o._req;
  }
}

Response& Response::operator=(const Response& o)
{
  if (this != &o) {
    _status       = o._status;
    _reqline      = o._reqline;
    _cli          = o._cli;
    _vsrv         = o._vsrv;
    _respoHeaders = o._respoHeaders;
    _body         = o._body;
    _mimeType     = o._mimeType;
    _respoStr     = o._respoStr;
    _closeConn    = o._closeConn;
    _req          = o._req;
  }
  return (*this);
}

Response::~Response() {}

// ------------------------------=[ END OCF ]=------------------------------ //

// FIXME maybe refac this to use req directly. depends on how often these will
// be used later in code
void Response::_setFieldsFromReq(Request& req)
{
  _status       = req.getStatusCode();
  _reqline      = req.getReqline();
  _cli          = req.getCli();
  _vsrv         = req.getVsrv();
  _closeConn    = req.closeConn();
  _matchedRoute = req.getMatchedRoute();
  _targetPath   = req.getTargetPath();
  _req          = &req;
}

// THE main function for finally doing what the request wants. When we finally
// arrive here we have the following setting:
//
//  1) status == 200: everything fine until here do normally processing of
//     request
//
//  2) status == 400: we had a bad request. therefore the only thing left to do
//     is finding the corresponding error page from vsrv BUT keep in mind that
//     client could be virtual!
//
e_HTTPStatus Response::generateResponse(Request& req)
{
  _setFieldsFromReq(req);

  // handle HTTP_400

  if (req.badRequest()) {
    Logger::logBug("Response::generateResponse", "Bad Request handling");
    _handleBadRequest();
  }

  // hrom here on: only HTTP_200 so far

  else if (req.isRedir()) {
    Logger::logBug("Response::generateResponse", "Redir Request handling");
    _handleRedir();
  }
  else if (req.isCGI()) {
    Logger::logBug("Response::generateResponse", "CGI Request handling");
    _handleCGI();
  }
  else if (req.isSimplePOST()) {
    Logger::logBug("Response::generateResponse", "SimplePost Request handling");
    _handleSimplePost();
  }
  else if (req.isDELETE()) {
    Logger::logBug("Response::generateResponse", "DELETE Request handling");
    _handleDelete();
  }
  // simple GET request
  else {
    Logger::logBug("Response::generateResponse", "Normal GET Request handling");
    _getBody200();
  }

  _buildRespoHdrs();
  _genResponse();

  return _status;
}

// @brief This is the core function for generating the final respostr. Extracted
// this in order to avoid codedup in genErrResponse().
void Response::_genResponse()
{
  for (std::map<str, str>::reverse_iterator it = _respoHeaders.rbegin();
      it != _respoHeaders.rend();
      it++)
    if (it->first == "Startline")
      _respoStr += it->second + CRLF;
    else
      _respoStr += it->first + ": " + it->second + CRLF;

  _respoStr += CRLF + _body;
}

// FIXME: this is not at done yet!
// TODO: we need to handle real routing in here.
// FIXME: `root` fields in config should never end in a '/'
void Response::_getBody200()
{
  const Route& r = *_matchedRoute;

  // root will NEVER have a trailing slash by parsing!
  str path = r.getRoot();
  Logger::logBug("root: " + path);

  // At this point there can only be single slashes in _targetPath as they have
  // been compressed by URL class.
  if (!_targetPath.empty()) {
    if (_targetPath[0] == '/')
      path += _targetPath;
    else
      path += "/" + _targetPath;
  }

  if (isDir(path) == 1)
    path += (path[path.size() - 1] == '/' ? "" : "/") + r.getIndex();
  Logger::log_dbg1("Response::_getBody: trying to read from file: " + path);

  _readBodyFromFile(path);
}

void Response::_buildRespoHdrs()
{
  _respoHeaders["Startline"] = "HTTP/1.1 " + WsrvLib::getStatusStr(_status);
  _respoHeaders["Server"]    = "m0fr1m's webserv " VERSION;

  // FIXME: maybe use sth else here
  _respoHeaders["Date"] = Logger::getLogtime();

  _respoHeaders["Content-Type"] =
      (_status == HTTP_200) ? _mimeType : "text/html";

  _respoHeaders["Content-Length"] = int2str(_body.size());

  if (_req->isRedir())
    _respoHeaders["Location"] = "http://" + _req->getHeaders()["host"] +
        _req->getRedir().second + _req->getTargetPath() +
        (_req->getReqline().target.getQuery().empty() ? "" : "?") +
        _req->getReqline().target.getQueryStr();

  str conn;
  if ((_status >= HTTP_400 && (_status != HTTP_404)) || _closeConn)
    conn = "close";
  else
    conn = "keep-alive";
  _respoHeaders["Connection"] = conn;

  _respoHeaders["Accept-Ranges"] = "none";
}

str Response::getRespoStr() const { return _respoStr; }

// reset.
void Response::reset()
{
  _status              = HTTP_200;
  _reqline.httpVersion = HTTPVER_UNKNOWN;
  _matchedRoute        = NULL;
  _reqline.method      = M_UNKNOWN;
  _req                 = NULL;

  _reqline.target.clear();
  _respoHeaders.clear();
  _body.clear();
  _mimeType.clear();
  _respoStr.clear();
  _targetPath.clear();
}

// helper function for generating the response for a failes Req.
std::map<str, str> Response::_buildErrRespoHdrs(
    e_HTTPStatus status, const str& body)
{
  std::map<str, str> hdrs;
  hdrs["Startline"]      = "HTTP/1.1 " + WsrvLib::getStatusStr(status);
  hdrs["Server"]         = "m0fr1m's webserv " VERSION;
  hdrs["Date"]           = Logger::getLogtime();
  hdrs["Content-Type"]   = "text/html";
  hdrs["Content-Length"] = int2str(body.length());

  str conn;
  if (status >= HTTP_400 && (status != HTTP_404))
    conn = "close";
  else
    conn = "keep-alive";
  hdrs["Connection"] = conn;
  return hdrs;
}

// Returns the response string to a given error status code.
str Response::genDefaultErrResponse(e_HTTPStatus errCode, constr errPage)
{
  str respostr;
  str body;

  if (!errPage.empty())
    body = errPage;
  else
    body = WsrvLib::getDefaultErrPage(errCode);

  std::map<str, str> hdrs = _buildErrRespoHdrs(errCode, body);

  for (std::map<str, str>::reverse_iterator it = hdrs.rbegin();
      it != hdrs.rend();
      it++)
    if (it->first == "Startline")
      respostr += it->second + CRLF;
    else
      respostr += it->first + ": " + it->second + CRLF;
  respostr += CRLF + body;
  return respostr;
}

// MASSIVE TODO !!!!!

// Handle the case of bad req. The only thing left to do in here is checking if
// client is still virtual -> _body = default errpage. If not virtual check if
// there is an errPage in server scope and try to read it. If that fails -> set
// status 404 and return default 404. Else return default 400.
void Response::_handleBadRequest()
{
  if (_cli->isVirtual()) {
    _body = WsrvLib::getDefaultErrPage(_status);
    return;
  }

  str errPage = _vsrv->getErrPage(_status);

  if (errPage.empty())
    _body = WsrvLib::getDefaultErrPage(_status);
  else
    _readBodyFromFile(_vsrv->getRoot() + errPage);
}

// get status page first from matched route secondly from vsrv. fallback to
// default page if both did not work
//
// TODO to be very perfect here in _readBodyFromFile we would also have to
// return the corresponding 404 if a status page could not be found.
void Response::_handleRedir()
{
  const e_HTTPStatus& redirCode = _matchedRoute->getRedir().first;

  _status = redirCode;

  str errPage = _matchedRoute->getErrPage(redirCode);
  if (errPage.empty())
    errPage = _vsrv->getErrPage(redirCode);
  else
    _readBodyFromFile(_matchedRoute->getRoot() + errPage);

  if (errPage.empty())
    _body = WsrvLib::getDefaultErrPage(redirCode);
  else
    _readBodyFromFile(_vsrv->getRoot() + errPage);
}

void Response::_handleCGI() {}

void Response::_handleSimplePost() {}

void Response::_handleDelete() {}

void Response::_readBodyFromFile(constr& path)
{
  _mimeType = WsrvLib::getMimeTypeFromPath(path);

  std::ifstream target;

  if (_mimeType != "text/html")
    target.open(path.c_str(), std::ios_base::binary);
  else
    target.open(path.c_str());

  if (!target) {
    _status = HTTP_404;
    _body   = WsrvLib::getDefaultErrPage(HTTP_404);
    return;
  }

  // get length of file:
  target.seekg(0, target.end);
  int length = target.tellg();
  target.seekg(0, target.beg);

  // FIXME: check length
  // QUESTION: do we still need it here?
  if (length <= 0) {
    _status = HTTP_404;
    _body   = WsrvLib::getDefaultErrPage(_status);
    return;
  }

  Logger::logDbg1("Response::_getBody", "body-length = " + int2str(length));

  // QUESTION: is this really the optimal solution?
  // FIXME: what about binary data?
  std::vector<char> buffer(length);

  // good2know: does not throw but sets the badbit
  target.read(&buffer[0], length);

  if (target)
    // like this even NUL bytes and other weird binary-mode data is written to
    // the std::string obj.
    //
    // Furthermore std::string::assign might throw exceptions if the assigned
    // data is too large. So we catch it here
    try {
      _body.assign(buffer.begin(), buffer.end());
    } catch (const std::exception& e) {
      Logger::log_err("Response::_getBody", "Read body too large!");
      _status = HTTP_413;
      _body   = WsrvLib::getDefaultErrPage(_status);
      return;
    }
}
