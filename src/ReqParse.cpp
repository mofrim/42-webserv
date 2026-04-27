/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReqParse.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 15:06:21 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/27 16:39:11 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "ReqParse.hpp"
#include "WsrvLib.hpp"
#include "utils.hpp"

// --------------------------------=[ OCF ]=-------------------------------- //

ReqParse::ReqParse()
{}

ReqParse::ReqParse(const ReqParse& other)
{
  (void)other;
}

ReqParse& ReqParse::operator=(const ReqParse& other)
{
  (void)other;
  return (*this);
}

ReqParse::~ReqParse()
{}

// ------------------------------=[ END OCF ]=------------------------------ //

int ReqParse::parseReqLine(t_RequestLine& rl, const str& rlstr)
{
  if (rlstr.size() > MAX_REQLINE_LEN)
    return KO;

  int i = 0;
  while (i < 6 && !std::isspace(rlstr[i]))
    i++;
  Logger::log_dbg1("ReqParse: found this method: '" + rlstr.substr(0, i) + "'");
  if ((rl.method = str2method(rlstr.substr(0, i))) == M_UNKNOWN)
    return HTTP_400;

  int k = 0;
  i++;
  while (k <= MAX_TARGET_LEN && !std::isspace(rlstr[i + k]))
    k++;
  Logger::log_dbg1(
      "ReqParse: found this target URL: '" + rlstr.substr(i, k) + "'");
  if (k > MAX_TARGET_LEN)
    return HTTP_400;
  rl.target = rlstr.substr(i, k);
  if (validateUrl(rl.target) == KO)
    return HTTP_400;

  i = i + k + 1;
  k = 0;
  while (i + k <= MAX_REQLINE_LEN && !std::isspace(rlstr[i + k]))
    k++;
  if (i + k > MAX_REQLINE_LEN)
    return HTTP_400;
  Logger::log_dbg1(
      "ReqParse: found this httpVer: '" + rlstr.substr(i, k) + "'");
  rl.httpVersion = rlstr.substr(i, k);
  if (rlstr.compare(i + k, 2, CRLF) != 0)
    return HTTP_400;

  return HTTP_200;
}

// TODO: add more validity checks here
std::vector< std::pair<str, str> > ReqParse::splitHdr(const str& hstr)
{
  std::vector< std::pair<str, str> > ret;

  // isolate headers from body
  str              onlyHdrs = hstr.substr(0, hstr.find(CRLFX2) + 2);
  std::vector<str> hdrLines = splitString(onlyHdrs, CRLF);

  for (std::vector<str>::iterator it = hdrLines.begin(); it != hdrLines.end();
      it++)
  {
    size_t colonPos = it->find(":");
    if (colonPos == str::npos)
      throw std::runtime_error("ReqParse::spliHdr: ivalid header field");
    str fieldName  = it->substr(0, colonPos);
    str fieldValue = strip(it->substr(colonPos + 1, str::npos));
    ret.push_back(std::make_pair(fieldName, fieldValue));
  }
  return ret;
}

int ReqParse::parseHeaders(std::map<str, str>& _headers, const str& reqstr)
{
  str              onlyHdrs = reqstr.substr(0, reqstr.find(CRLFX2) + 2);
  std::vector<str> hdrLines = splitString(onlyHdrs, CRLF);

  // skipping the requline
  std::vector<str>::iterator it = hdrLines.begin() + 1;
  for (; it != hdrLines.end(); it++) {
    size_t colonPos = it->find(":");
    if (colonPos == str::npos)
      return HTTP_400;
    str fieldName       = it->substr(0, colonPos);
    str fieldValue      = strip(it->substr(colonPos + 1, str::npos));
    _headers[fieldName] = fieldValue;
  }
  return HTTP_200;
}

// TODO: implement
int ReqParse::validateUrl(const str& u)
{
  (void)u;
  return OK;
}
