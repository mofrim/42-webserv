/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request_Parsing.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 18:46:40 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/13 12:51:08 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Request.hpp"
#include "VServer.hpp"
#include "WsrvLib.hpp"
#include "utils.hpp"

// FIXME add header / body separation somewhere around / before here
e_HTTPStatus Request::parseReqHeaders()
{
  if ((_statusCode = _parseReqLine()) == HTTP_400) {
    Logger::log_dbg1("Request::parseReqline: 400");
    return _statusCode;
  }
  if ((_statusCode = _parseHeaders()) == HTTP_400) {
    Logger::log_dbg1("Request::_parseHeaders: 400");
    return _statusCode;
  }
  if ((_statusCode = _evaluateHdrs()) == HTTP_400) {
    Logger::log_dbg1("Request::checkHeaders: 400");
    return _statusCode;
  }
  return HTTP_200;
}

e_HTTPStatus Request::_parseReqLine()
{
  e_HTTPStatus status = HTTP_200;
  if ((status = _readReqline()) == HTTP_400)
    return status;
  _requestTarget = _reqline.target.getPath();

  return status;
}

// Parse the reuqest-line. I.e. the uppercase method, then a target URI and then
// the HTTP-version. The URI is already converted to to lowercase
e_HTTPStatus Request::_readReqline()
{
  if (_reqstr.size() > MAX_REQLINE_LEN)
    return HTTP_400;

  int i = _skipEmptyHdrLine();
  int k = 0;
  while (k < 6 && !std::isspace(_reqstr[i + k]))
    k++;
  Logger::log_dbg1(
      "Request: found this method: '" + _reqstr.substr(i, k) + "'");
  if ((_reqline.method = str2meth(_reqstr.substr(i, k))) == M_UNKNOWN)
    return HTTP_400;

  i = i + k + 1;
  k = 0;
  while (k <= MAX_TARGET_LEN && !std::isspace(_reqstr[i + k]))
    k++;
  Logger::log_dbg1(
      "Request: found this target URL: '" + _reqstr.substr(i, k) + "'");
  if (k > MAX_TARGET_LEN)
    return HTTP_400;
  _reqline.target.parseTargetURL(_reqstr.substr(i, k));
  if (_reqline.target.bad())
    return HTTP_400;

  Logger::logDbg1(
      "Request::_readReqline", "URL path: " + _reqline.target.getPath());
  Logger::logDbg1(
      "Request::_readReqline", "URL query: " + _reqline.target.getQueryAsStr());

  i = i + k + 1;
  k = 0;
  while (i + k <= MAX_REQLINE_LEN && !std::isspace(_reqstr[i + k]))
    k++;
  if (i + k > MAX_REQLINE_LEN)
    return HTTP_400;
  Logger::log_dbg1(
      "Request: found this httpVer: '" + _reqstr.substr(i, k) + "'");
  _reqline.httpVersion = WsrvLib::str2HTTPVer(_reqstr.substr(i, k));
  if (_reqstr.compare(i + k, 2, CRLF) != 0)
    return HTTP_400;

  // FIXME: add here or somwhere else a NGINX like logmsg
  return HTTP_200;
}

// Parse all hdrs lowercasing the field names because we are case-insensitive by
// RFC. Also we strip any leading or trailing whitespaces from names and values.
e_HTTPStatus Request::_parseHeaders()
{
  str              onlyHdrs = _reqstr.substr(0, _reqstr.find(CRLFX2) + 2);
  std::vector<str> hdrLines = splitString(onlyHdrs, CRLF);

  // skipping the requline
  std::vector<str>::iterator it = hdrLines.begin() + 1;
  for (; it != hdrLines.end(); it++) {
    size_t colonPos = it->find(":");
    if (colonPos == str::npos)
      return HTTP_400;
    str fieldName  = strip(it->substr(0, colonPos));
    str fieldValue = strip(it->substr(colonPos + 1, str::npos));
    tolower(fieldName);
    _headers[fieldName] = fieldValue;
  }
  return HTTP_200;
}

// Checks headers and Request Semantics. I.e.if req != POST the body will
// automatically be set to complete.
e_HTTPStatus Request::_evaluateHdrs()
{
  if (_reqline.httpVersion == HTTPVER_1_1) {
    if (_reqline.method == M_GET) {

      // RFC MUST for HTTP/1.1
      if (_headers.count("host") == 0) {
        Logger::log_srv(_vsrv->getName(), "GET Req without Host header", WARN);
        return HTTP_400;
      }
      if (_headers.count("connection") > 0 && _headers["connection"] == "close")
        _closeConn = true;
    }
  }
  else {
    if (!(_headers.count("connection") > 0 &&
            _headers["connection"] == "keep-alive"))
    {
      Logger::log_dbg1(
          "Request::checkHeaders: closing conn per default, reason: HTTP <= "
          "1.0");
      _closeConn = true;
    }
  }

  // we simply ignore the body if we ain't got no POST. by setting _bodyComplete
  // the Request::reqComplete will return true bc _hdrComplete is already true
  if (_reqline.method != M_POST)
    _bodyComplete = true;

  // FIXME: in theory content-length == 0 will be okay but not very useful.
  if (_reqline.method == M_POST) {

    if (_headers.count("content-length") == 0) {
      Logger::log_srv(_vsrv->getName(), "No content with POST req", WARN);
      return HTTP_400;
    }

    // TODO
    // NEXT add handling of req body in here
  }

  return HTTP_200;
}

// we skip exactly one empty line prior to startline as RFC friendly asks from
// us.
size_t Request::_skipEmptyHdrLine() const
{
  if (_reqstr.size() >= 2 && !_reqstr.compare(0, 2, CRLF))
    return 2;
  return 0;
}

// TODO: add more validity checks here
std::vector< std::pair<str, str> > Request::_splitHdr()
{
  std::vector< std::pair<str, str> > ret;

  // isolate headers from body
  str              onlyHdrs = _reqstr.substr(0, _reqstr.find(CRLFX2) + 2);
  std::vector<str> hdrLines = splitString(onlyHdrs, CRLF);

  for (std::vector<str>::iterator it = hdrLines.begin(); it != hdrLines.end();
      it++)
  {
    size_t colonPos = it->find(":");
    if (colonPos == str::npos)
      throw std::runtime_error("Request::spliHdr: ivalid header field");
    str fieldName  = it->substr(0, colonPos);
    str fieldValue = strip(it->substr(colonPos + 1, str::npos));
    ret.push_back(std::make_pair(fieldName, fieldValue));
  }
  return ret;
}
