/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ReqParse.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 15:06:21 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/24 17:36:57 by fmaurer          ###   ########.fr       */
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
  Logger::log_dbg0("ReqParse: found this method: '" + rlstr.substr(0, i) + "'");
  if ((rl.method = str2method(rlstr.substr(0, i))) == M_UNKNOWN)
    return HTTP_400;

  int k = 0;
  i++;
  while (k <= MAX_TARGET_LEN && !std::isspace(rlstr[i + k]))
    k++;
  Logger::log_dbg0(
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
  Logger::log_dbg0(
      "ReqParse: found this httpVer: '" + rlstr.substr(i, k) + "'");
  rl.httpVersion = rlstr.substr(i, k);
  if (rlstr.compare(i + k, 2, CRLF) != 0)
    return HTTP_400;

  return HTTP_200;
}

// TODO: implement
int ReqParse::validateUrl(const str& u)
{
  (void)u;
  return OK;
}
