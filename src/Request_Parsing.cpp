/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request_Parsing.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/01 18:46:40 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/03 23:35:47 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Request.hpp"
#include "VServer.hpp"
#include "WsrvLib.hpp"
#include "utils.hpp"

// Parse the reuqest-line. I.e. the uppercase method, then a target URI and then
// the HTTP-version. The URI is already converted to to lowercase
e_HTTPStatus Request::_readReqline()
{
  if (_reqstr.size() > MAX_REQLINE_LEN)
    return HTTP_400;

  int i = 0;
  while (i < 6 && !std::isspace(_reqstr[i]))
    i++;
  Logger::log_dbg1(
      "Request: found this method: '" + _reqstr.substr(0, i) + "'");
  if ((_reqline.method = str2method(_reqstr.substr(0, i))) == M_UNKNOWN)
    return HTTP_400;

  int k = 0;
  i++;
  while (k <= MAX_TARGET_LEN && !std::isspace(_reqstr[i + k]))
    k++;
  Logger::log_dbg1(
      "Request: found this target URL: '" + _reqstr.substr(i, k) + "'");
  if (k > MAX_TARGET_LEN)
    return HTTP_400;
  _reqline.target = _reqstr.substr(i, k);
  tolower(_reqline.target);
  if (validateUrl(_reqline.target) == KO)
    return HTTP_400;

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

  return HTTP_200;
}

e_HTTPStatus Request::parseReqLine()
{
  e_HTTPStatus status = HTTP_200;
  if ((status = _readReqline()) >= HTTP_400)
    return status;
  // if ((status = validateUrl(rl.target)) >= HTTP_400)
  //   return status;

  return status;
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

e_HTTPStatus Request::checkHeaders()
{
  if (_reqline.httpVersion == HTTPVER_1_1) {
    if (_reqline.method == M_GET) {
      if (_headers.find("host") == _headers.end()) {
        Logger::log_srv(_vsrv->getName(), "GET Req without Host header", WARN);
        return HTTP_400;
      }
    }
  }
  return HTTP_200;
}

// TODO: implement
int Request::validateUrl(const str& u)
{
  (void)u;
  return OK;
}
