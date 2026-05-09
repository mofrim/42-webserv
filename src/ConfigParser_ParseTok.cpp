/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser_ParseTok.cpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/08 21:14:52 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/10 00:12:15 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include "Logger.hpp"
#include "utils.hpp"

// ------------------------=[ Server Scope Parsing ]=------------------------ //

// -------------------------------=[ Utils ]=-------------------------------- //

static bool isValidPathChar(char c)
{
  return isalnum(c) || c == '.' || c == '-' || c == '_' || c == '/';
}

static bool isValidDomainChar(char c)
{
  return isalnum(c) || c == '.' || c == '-' || c == '_';
}

// we never want trailing '/'
static bool isValidRoute(const str& s)
{
  if (s.length() == 0)
    return false;

  str::const_iterator it = s.begin();
  if (*it != '/' || (s.size() > 1 && *(s.end() - 1) == '/'))
    return false;

  for (; it != s.end(); ++it)
    if (!isValidPathChar(*it))
      return false;

  return true;
}

static bool isValidDomainName(const str& n)
{
  constrIt it = n.begin();
  if (!isalnum(*it))
    return false;
  for (; it != n.end(); ++it)
    if (!isValidDomainChar(*it))
      return false;
  return true;
}

// ---------------------------=[ The Real Stuff ]=--------------------------- //

// serverName
bool ConfigParser::_parseTokName(VServerCfg& vcfg)
{
  if ((++_tokIt)->type != TOK_NAME)
    throw std::runtime_error("Wrong token while parsing serverName");

  if (!isValidDomainName(_tokIt->val)) {
    Logger::log_warn("cfg line " + int2str(_tokIt->line),
        "serverName has to be valid domain-name!");
    return false;
  }

  vcfg.setServerName(_tokIt->val);

  return true;
}

// route
// TODO: checking where?
bool ConfigParser::_parseTokRoute()
{
  if ((++_tokIt)->type != TOK_ROUTE)
    throw std::runtime_error("Wrong token while parsing route");

  _currentRoute.reset();

  if (!isValidRoute(_tokIt->val)) {
    Logger::log_warn("cfg line " + int2str(_tokIt->line), "Invalid route!");
    while (_tokIt != _tokens.end() && _tokIt->type != TOK_BEND)
      ++_tokIt;
    return false;
  }

  _currentRoute.setPath(_tokIt->val);
  _scope.push(S_ROUTE);
  return true;
}

// listen
bool ConfigParser::_parseTokIface(VServerCfg& vcfg)
{
  if ((++_tokIt)->type != TOK_IFACE)
    throw std::runtime_error("Wrong token while parsing iface");

  std::vector<str> split = splitString(_tokIt->val, ":");

  if (split.size() != 2) {
    Logger::log_warn(
        "cfg line " + int2str(_tokIt->line), "Invalid interface value");
    return false;
  }

  strip(split[0]);
  strip(split[1]);

  if (!isValidDomainName(split[0])) {
    Logger::log_warn("cfg line " + int2str(_tokIt->line),
        "invalid iface addr - ip or domain!)");
    return false;
  }

  vcfg.addInterface(split[0], str2u16(split[1]));

  return true;
}

// Rulez:
//  - format: STATUS:PATH
//  - example: 404:/404.html, 400:/err/400.html
//  - have to start with slash
//  - will be searched under corresponding root
bool ConfigParser::_parseTokError(VServerCfg& vcfg)
{
  if ((++_tokIt)->type != TOK_ERROR)
    throw std::runtime_error("Wrong token while parsing errorPage");

  std::vector<str> split = splitString(_tokIt->val, ":");

  if (split.size() != 2) {
    Logger::log_warn("cfg line " + int2str(_tokIt->line), "Invalid errorPage!");
    return false;
  }

  strip(split[0]);
  strip(split[1]);

  e_HTTPStatus errCode = WsrvLib::short2HttpStatus(str2u16(split[0]));

  if (errCode < 400) {
    Logger::log_warn(
        "cfg line " + int2str(_tokIt->line), "Invalid error status code!");
    return false;
  }

  if (!isValidRoute(split[1])) {
    Logger::log_warn("cfg line " + int2str(_tokIt->line),
        "Invalid file route for errorPage!");
    return false;
  }

  if (_scope.top() == S_SERVER)
    vcfg.addErrPage(errCode, split[1]);
  else
    _currentRoute.addErrPage(errCode, split[1]);

  return true;
}

// ------------------------=[ Route Scope Parsing ]=------------------------- //

// index
bool ConfigParser::_parseTokFname()
{
  ++_tokIt;
  return true;
}
// autoindex
bool ConfigParser::_parseTokBool()
{
  ++_tokIt;
  return true;
}
// methods
bool ConfigParser::_parseTokMeth()
{
  ++_tokIt;
  return true;
}
// redirect
bool ConfigParser::_parseTokRedir()
{
  ++_tokIt;
  return true;
}
// cgisdada
bool ConfigParser::_parseTokCgi()
{
  ++_tokIt;
  return true;
}

// ------------------------=[ Mixed Scope Parsing ]=------------------------- //

// maxBodySize
bool ConfigParser::_parseTokBytes(VServerCfg& vcfg)
{
  (void)vcfg;
  ++_tokIt;
  return true;
}

// root, upload
bool ConfigParser::_parseTokFspath(VServerCfg& vcfg)
{
  (void)vcfg;
  ++_tokIt;
  return true;
}
