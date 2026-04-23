/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 19:11:25 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/23 22:56:02 by fmaurer          ###   ########.fr       */
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
      _respoStr += it->second + "\r\n";
    else
      _respoStr += it->first + ": " + it->second + "\r\n";

  _respoStr += "\r\n" + _body;
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
    std::ifstream target(path.c_str());

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

    if (length <= 0) {
      _statusCode = HTTP_404;
      _body       = HttpStatus::getDefaultErrPage(HTTP_404);
      return;
    }

    Logger::log_dbg1("Response::_getBody: length = " + int2str(length));

    // FIXME: check length

    char *buffer = new char[length];
    target.read(buffer, length);
    if (target)
      _body = buffer;
    delete[] buffer;
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

  _respoHeaders["Content-Type"] = "text/html";

  _respoHeaders["Content-Length"] = int2str(_body.size());

  // QUESTION: what is this about? Nginx does it.
  _respoHeaders["Accept-Ranges"] = "bytes";
}

str Response::getStr() const
{
  return _respoStr;
}
