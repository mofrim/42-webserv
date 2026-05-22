/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request_Parsing.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 18:46:40 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/22 13:18:16 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Request.hpp"
#include "VServer.hpp"
#include "WsrvLib.hpp"
#include "utils.hpp"

#include <cstdlib>

// parse reqline and, if successful, match the route responsible for handling
// this req
e_HTTPStatus Request::parseReqLine()
{
  _reqlineParsed = true;
  if ((_statusCode = _readReqline()) == HTTP_400)
    return _statusCode;
  _requestTarget = _reqline.target.getPath();
  _target        = _reqline.target;

  Logger::logSrv(_vsrvName, "Reqline: '" + _getReqlineAsStr() + "'");

  // if our client is not virtual immediately evaluate the target path
  if (!_cli->isVirtual())
    this->evaluateTarget();

  return _statusCode;
}

void Request::evaluateTarget()
{
  if (_cli->getVsrv() == NULL)
    throw std::runtime_error(
        "(Request::_evaluateTarget) virtual client cannot eval target!");

  this->_matchRoute();

  Logger::logSrv(_vsrvName, "-> Matched Route: " + _matchedRoute->getPath());
  Logger::logSrv(_vsrvName, "-> Target Path: " + _targetPath);
  Logger::logSrv(_vsrvName, "-> Query: " + _target.getQueryCSStr());

  // check if method is allowed for this route, if not -> 403
  const std::set<e_Method>& allowedMethods = _matchedRoute->getMethods();
  if (allowedMethods.find(_reqline.method) == allowedMethods.end()) {
    Logger::logSrv(_vsrvName,
        "forbidden meth " + meth2str(_reqline.method) + " for route " +
            _matchedRoute->getPath());
    _statusCode = HTTP_403;
    return;
  }

  // classify request
  _isCGI        = (!_matchedRoute->getCgi().empty());
  _isSimplePOST = (!_isCGI && _reqline.method == M_POST);
  _isDELETE     = (_reqline.method == M_DELETE);

  _isRedir = (_matchedRoute->getRedir().first != HTTP_0);
  if (_isRedir)
    _redir = _matchedRoute->getRedir();
}

// is only being called if CRLFX2 was found in reqdata! That means: we can
// separate body and headers from here on.
e_HTTPStatus Request::parseReqHeaders()
{
  _hdrsParsed = true;

  if ((_statusCode = _parseHeaders()) >= HTTP_400) {
    Logger::logDbg1("Request::_parseHeaders: 400");
    return _statusCode;
  }
  if ((_statusCode = _evaluateHdrs()) >= HTTP_400) {
    Logger::logDbg1("Request::checkHeaders: 400");
    return _statusCode;
  }
  return HTTP_200;
}

// Parse the reqline. I.e. the uppercase method, then a target URI and then
// the HTTP-version. I leave it some kind of arbitrary threshold for of 2 *
// MAX_REQLINE_LEN to successfully send a valid reqline. but if there is only
// garbage in _reqdata it will fail anyhow in one of the next steps.
e_HTTPStatus Request::_readReqline()
{
  if (_reqdata.size() > 2 * MAX_REQLINE_LEN)
    return HTTP_400;

  int i = _skipEmptyHdrLine();
  int k = 0;
  while (k < 6 && !std::isspace(_reqdata[i + k]))
    k++;
  Logger::logDbg1("Request::_readReqline",
      "Request: found this method: '" + _reqdata.substr(i, k) + "'");
  if ((_reqline.method = str2meth(_reqdata.substr(i, k))) == M_UNKNOWN) {
    Logger::logSrv(_vsrvName,
        "Invalid method in Reqline: '" +
            data2hexStr(
                _reqdata.substr(i, k).data(), _reqdata.substr(i, k).size()) +
            "'");
    return HTTP_400;
  }

  i = i + k + 1;
  k = 0;
  while (k <= MAX_TARGET_LEN && !std::isspace(_reqdata[i + k]))
    k++;
  Logger::logDbg1("Request::_readReqline",
      "Request: found this target URL: '" + _reqdata.substr(i, k) + "'");
  if (k > MAX_TARGET_LEN)
    return HTTP_400;
  _reqline.target.parsePath(_reqdata.substr(i, k));
  if (_reqline.target.bad())
    return HTTP_400;

  Logger::logDbg1(
      "Request::_readReqline", "URL path: " + _reqline.target.getPath());
  Logger::logDbg1(
      "Request::_readReqline", "URL query: " + _reqline.target.getQueryCSStr());

  i = i + k + 1;
  k = 0;
  while (i + k <= MAX_REQLINE_LEN && !std::isspace(_reqdata[i + k]))
    k++;
  if (i + k > MAX_REQLINE_LEN)
    return HTTP_400;
  Logger::logDbg1("Request::_readReqline",
      "Request: found this httpVer: '" + _reqdata.substr(i, k) + "'");
  _reqline.httpVersion = WsrvLib::str2HTTPVer(_reqdata.substr(i, k));
  if (_reqdata.compare(i + k, 2, CRLF) != 0)
    return HTTP_400;

  return HTTP_200;
}

// convenience func for logging some maybe interesting hdrs
void Request::_logSelectedHdrs()
{
  str logstr("-> Some Hdrs: ");
  if (_headers.find("host") != _headers.end())
    logstr += "Host='" + _headers["host"] + "'";
  if (_headers.find("user-agent") != _headers.end()) {
    logstr += " -- ";
    logstr += "UserAgent='" + _headers["user-agent"] + "'";
  }
  if (_headers.find("content-type") != _headers.end()) {
    logstr += " -- ";
    logstr += "ContentType='" + _headers["content-type"] + "'";
  }
  Logger::logSrv(_vsrvName, logstr);
}

// Parse all hdrs lowercasing the field names because we are case-insensitive by
// RFC. Also we strip any leading or trailing whitespaces from names and values.
// The hdrs / body separation for POST reqs is also handled in here bc we search
// fo CRLFX2 anyway and we would have to do this again elsewhere otherwise.
e_HTTPStatus Request::_parseHeaders()
{
  Logger::logDbg1("Request::_parseHeaders", "Parsing hdrs...");
  size_t crlfx2 = _reqdata.find(CRLFX2);

  if (crlfx2 == str::npos)
    throw std::runtime_error(
        "(Request::_parseHeaders) somehow said to be finished headers weren't "
        "finished :/");

  // +2 because we want to keep the CRLF after the last hdr line
  str onlyHdrs = _reqdata.substr(0, crlfx2 + 2);

  std::vector<str> hdrLines = splitString(onlyHdrs, CRLF);

  // skipping the requline
  // apllying RFC MUST rule here: no CR anywhere in the header!
  std::vector<str>::iterator it = hdrLines.begin() + 1;
  for (; it != hdrLines.end(); ++it) {
    size_t colonPos = it->find(":");
    if (colonPos == str::npos || it->find("\n") != str::npos)
      return HTTP_400;
    str fieldName  = strip(it->substr(0, colonPos));
    str fieldValue = strip(it->substr(colonPos + 1, str::npos));
    toLowerInPlace(fieldName);
    _headers[fieldName] = fieldValue;
  }

  // handle hdrs body separation for POST reqs. we also keep track of the
  // bodySize in main Request class bc we might have to handle socket draining
  // using it. Therefore we have to track bodySize and compare with
  // content-length.
  e_HTTPStatus initBodyRet = HTTP_200;
  if (_reqline.method == M_POST)
    initBodyRet = _initializeBody(onlyHdrs, crlfx2);
  if (initBodyRet != HTTP_200)
    return initBodyRet;

  // set free _reqdata as we will not need it anymore.
  _reqdata.clear();

  _logSelectedHdrs();
  return HTTP_200;
}

// seperate the POST body from the headers so far read into _reqdata. Do some
// checks if content-length hdr field was set appropriately.
e_HTTPStatus Request::_initializeBody(constr& onlyHdrs, size_t crlfx2)
{

  if (_headers.count("content-length") == 0) {
    Logger::logSrv(_vsrvName, "No content with POST req", WARN);
    return HTTP_400;
  }

  _contentLength = std::atol(_headers["content-length"].c_str());

  if (_contentLength > MAX_CONTENT_LENGTH) {
    Logger::logSrv(_vsrvName,
        "Request Content-Length exceeds MAX_CONTENT_LENGTH -> 400",
        WARN);
    return HTTP_400;
  }

  if (_matchedRoute != NULL && _contentLength > _matchedRoute->getMaxBodySize())
  {
    Logger::logSrv(
        _vsrvName, "Requested Content-Length exceeds maxBodySize", WARN);
    _cli->setState(CLI_DRAIN);
    return HTTP_413;
  }

  // Setting body's capacity to _contentLength as we will ignore anything
  // above it. If, body's already has been too large here, this will truncate
  // the data.
  if (_body.setMaxSize(_contentLength) == KO)
    throw std::runtime_error(
        "(Request::_parseHeaders) Could not set maxBodySize!");

  _bodySize = _reqdata.size() - onlyHdrs.size() - 2;
  _body.appendData(_reqdata.substr(crlfx2).data() + 4, _bodySize);
  Logger::logDbg1("Request::_parseHeaders",
      "initialized body with " + int2str(_bodySize) + " bytes!");

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
        Logger::logSrv(_vsrvName, "GET Req without Host header", WARN);
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
      Logger::logDbg1(
          "Request::checkHeaders: closing conn per default, reason: HTTP <= "
          "1.0");
      _closeConn = true;
    }
  }

  // we simply ignore the body if we ain't got no POST. by setting _bodyComplete
  // the Request::reqComplete will return true bc _hdrComplete is already true
  if (_reqline.method != M_POST)
    _bodyComplete = true;

  // set host and hostPort (which is the servers addr and port) from hdrs
  if (!_headers["host"].empty()) {
    Logger::logBug("header(host): " + _headers["host"]);
    std::vector<str> hspl = splitString(_headers["host"], ":");
    if (hspl.size() == 2) {
      if (str2u16(hspl[1]) != _hostPort)
        _hostPort = str2u16(strip(hspl[1]));
    }
    if (hspl.size() >= 1)
      _host = strip(hspl[0]);
  }

  return HTTP_200;
}

// we skip exactly one empty line prior to startline as RFC friendly asks from
// us.
size_t Request::_skipEmptyHdrLine() const
{
  if (_reqdata.size() >= 2 && !_reqdata.compare(0, 2, CRLF))
    return 2;
  return 0;
}

// TODO: add more validity checks here
std::vector< std::pair<str, str> > Request::_splitHdr()
{
  std::vector< std::pair<str, str> > ret;

  // isolate headers from body
  str              onlyHdrs = _reqdata.substr(0, _reqdata.find(CRLFX2) + 2);
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
