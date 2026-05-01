/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 19:11:25 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/01 09:50:15 by fmaurer          ###   ########.fr       */
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

Response::Response()
{}

Response::Response(const Response& o)
{
  if (this != &o) {
    _statusCode   = o._statusCode;
    _reqline      = o._reqline;
    _reqHeaders   = o._reqHeaders;
    _cli          = o._cli;
    _vsrv         = o._vsrv;
    _respoHeaders = o._respoHeaders;
    _body         = o._body;
    _mimeType     = o._mimeType;
    _respoStr     = o._respoStr;
  }
}

Response& Response::operator=(const Response& o)
{
  if (this != &o) {
    _statusCode   = o._statusCode;
    _reqline      = o._reqline;
    _reqHeaders   = o._reqHeaders;
    _cli          = o._cli;
    _vsrv         = o._vsrv;
    _respoHeaders = o._respoHeaders;
    _body         = o._body;
    _mimeType     = o._mimeType;
    _respoStr     = o._respoStr;
  }
  return (*this);
}

Response::~Response()
{}

// ------------------------------=[ END OCF ]=------------------------------ //

// FIXME: maybe i do not need this at all
Response::Response(const Request& req)
{
  _setFieldsFromReq(req);
}

void Response::_setFieldsFromReq(const Request& req)
{
  _statusCode = req.getStatusCode();
  _reqline    = req.getReqline();
  _reqHeaders = req.getHeaders();
  _cli        = req.getCli();
  _vsrv       = req.getVsrv();
}

e_HTTPStatus Response::genResponse(const Request& req)
{
  _setFieldsFromReq(req);

  if (_statusCode == HTTP_200 && _reqline.method == M_GET)
    _getBody();

  _genResponse();

  return _statusCode;
}

// @brief This is the core function for generating the final respostr. Extracted
// this in order to avoid codedup in genErrResponse().
void Response::_genResponse()
{
  _buildRespoHdrs();

  for (std::map<str, str>::reverse_iterator it = _respoHeaders.rbegin();
      it != _respoHeaders.rend();
      it++)
    if (it->first == "Startline")
      _respoStr += it->second + CRLF;
    else
      _respoStr += it->first + ": " + it->second + CRLF;

  _respoStr += CRLF + _body;
}

void Response::_getBody()
{
  if (_vsrv->getRoutes().size() == 1 &&
      _vsrv->getRoutes().begin()->first == "/")
  {
    Route r = _vsrv->getRoutes().begin()->second;

    str root = r.getRoot();
    str path = root + _reqline.target;
    if (isDir(path))
      path += (path[path.size() - 1] == '/' ? "" : "/") + r.getDefaultFile();
    Logger::log_dbg1("Response::_getBody: trying to read from file: " + path);

    _mimeType = WsrvLib::getMimeTypeFromPath(path);

    std::ifstream target;

    // FIXME: mime types!!!
    if (_mimeType != "text/html")
      target.open(path.c_str(), std::ios_base::binary);
    else
      target.open(path.c_str());

    // FIXME: is this really not good if file could not be opened?
    if (!target) {
      _statusCode = HTTP_404;
      _body       = WsrvLib::getDefaultErrPage(HTTP_404);
      return;
    }

    // get length of file:
    target.seekg(0, target.end);
    int length = target.tellg();
    target.seekg(0, target.beg);

    // FIXME: check length
    // QUESTION: do we still need it here?
    if (length <= 0) {
      _statusCode = HTTP_404;
      _body       = WsrvLib::getDefaultErrPage(_statusCode);
      return;
    }

    Logger::log_dbg1("Response::_getBody: length = " + int2str(length));

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
        Logger::log_err("In Response::_getBody: read body too large!");
        _statusCode = HTTP_413;
        _body       = WsrvLib::getDefaultErrPage(_statusCode);
        return;
      }
  }
  else
    _body = "";
}

void Response::_buildRespoHdrs()
{
  _respoHeaders["Startline"] = "HTTP/1.1 " + WsrvLib::getStatusStr(_statusCode);
  _respoHeaders["Server"]    = "m0fr1m's webserv " VERSION;

  // FIXME: maybe use sth else here
  _respoHeaders["Date"] = Logger::getLogtime();

  _respoHeaders["Content-Type"] =
      (_statusCode == HTTP_200) ? _mimeType : "text/html";

  _respoHeaders["Content-Length"] = int2str(_body.size());

  str conn;
  if (_statusCode >= HTTP_400 && (_statusCode != HTTP_404))
    conn = "close";
  else
    conn = "keep-alive";
  _respoHeaders["Connection"] = conn;

  _respoHeaders["Accept-Ranges"] = "none";
}

str Response::getRespoStr() const
{
  return _respoStr;
}

// reset.
void Response::reset()
{
  _statusCode = HTTP_200;
  _reqline.httpVersion.clear();
  _reqline.target.clear();
  _reqline.method = M_GET;
  _reqHeaders.clear();
  _respoHeaders.clear();
  _body.clear();
  _mimeType.clear();
  _respoStr.clear();
}

// helper function for generating the response for a failes Req.
std::map<str, str> Response::_buildErrRespoHdrs(u16 status)
{
  std::map<str, str> hdrs;
  hdrs["Startline"]      = "HTTP/1.1 " + WsrvLib::getStatusStr(status);
  hdrs["Server"]         = "m0fr1m's webserv " VERSION;
  hdrs["Date"]           = Logger::getLogtime();
  hdrs["Content-Type"]   = "text/html";
  hdrs["Content-Length"] = "0";

  str conn;
  if (status >= HTTP_400 && (status != HTTP_404))
    conn = "close";
  else
    conn = "keep-alive";
  hdrs["Connection"] = conn;
  return hdrs;
}

// Returns the response string to a given error status code.
str Response::genErrResponse(u16 errCode)
{
  str respostr;

  std::map<str, str> hdrs = _buildErrRespoHdrs(errCode);

  str body = WsrvLib::getDefaultErrPage(errCode);

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
