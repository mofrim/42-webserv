/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 19:11:25 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/24 10:28:38 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpStatus.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "VServer.hpp"
#include "utils.hpp"

#include <fstream>

// --------------------------------=[ OCF ]=-------------------------------- //

Response::Response()
{}

Response::Response(const Response& o)
{
  if (this != &o) {
    _statusCode = o._statusCode;
    _reqline    = o._reqline;
    _reqHeaders = o._reqHeaders;
    _cli        = o._cli;
    _vsrv       = o._vsrv;
  }
}

Response& Response::operator=(const Response& o)
{
  if (this != &o) {
    _statusCode = o._statusCode;
    _reqline    = o._reqline;
    _reqHeaders = o._reqHeaders;
    _cli        = o._cli;
    _vsrv       = o._vsrv;
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

void Response::genResponse(const Request& req)
{
  _setFieldsFromReq(req);

  Logger::log_dbg0(
      "Response::genResponse: statusCode = " + int2str(_statusCode));
  if (_statusCode == HTTP_200 && _reqline.method == M_GET)
    _getBody();
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
    Logger::log_dbg1("Response::_getBody: trying to read from file: " + path);
    if (isDir(path))
      path += (path[path.size() - 1] == '/' ? "" : "/") + r.getDefaultFile();

    _mimeType = _getMimeType(path);

    std::ifstream target;

    // FIXME: mime types!!!
    if (_mimeType != "text/html")
      target.open(path.c_str(), std::ios_base::binary);
    else
      target.open(path.c_str());

    // FIXME: is this really not good if file could not be opened?
    if (!target) {
      _statusCode = HTTP_404;
      _body       = HttpStatus::getDefaultErrPage(HTTP_404);
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
      _body       = HttpStatus::getDefaultErrPage(HTTP_404);
      return;
    }

    Logger::log_dbg1("Response::_getBody: length = " + int2str(length));

    // QUESTION: is this really the optimal solution?
    // FIXME: what about binary data?
    std::vector<char> buffer(length);
    target.read(&buffer[0], length);
    if (target)
      _body.assign(buffer.begin(), buffer.end());
  }
  else
    _body = "";
}

void Response::_buildRespoHdrs()
{
  _respoHeaders["Startline"] =
      "HTTP/1.1 " + HttpStatus::getStatusStr(_statusCode);
  _respoHeaders["Server"] = "m0fr1m's webserv " VERSION;

  // FIXME: maybe use sth else here
  _respoHeaders["Date"] = Logger::getLogtime();

  _respoHeaders["Content-Type"] =
      (_statusCode == HTTP_200) ? _mimeType : "text/html";

  _respoHeaders["Content-Length"] = int2str(_body.size());

  _respoHeaders["Connection"] = "close";

  // QUESTION: what is this about? Nginx does it.
  _respoHeaders["Accept-Ranges"] = "bytes";
}

str Response::getStr() const
{
  return _respoStr;
}

str Response::_getMimeType(const str& p)
{
  int i   = p.rfind(".") + 1;
  str ext = p.substr(i);

  if (ext == "html")
    return "text/html";

  if (ext == "jpg")
    return "image/jpeg";

  if (ext == "png")
    return "image/png";

  return "*/*";
}
