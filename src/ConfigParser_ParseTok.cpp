/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser_ParseTok.cpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/08 21:14:52 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/13 15:09:23 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include "Logger.hpp"
#include "utils.hpp"

#include <cstdlib>

// ------------------------=[ Server Scope Parsing ]=------------------------ //

// -------------------------------=[ Utils ]=-------------------------------- //

static bool isNumStr(const str& s)
{
  for (std::string::const_iterator it = s.begin(); it != s.end(); ++it)
    if (!isdigit(*it))
      return false;
  return true;
}

static bool isValidPathChar(char c)
{
  return isalnum(c) || c == '.' || c == '-' || c == '_' || c == '/';
}

static bool isValidFnameChar(char c)
{
  return isalnum(c) || c == '.' || c == '-' || c == '_';
}

// arbitraily deciding the filenames should not begin with '-'. idk, but that
// seems fishy to me.
static bool isValidFname(const str& s)
{
  if (s.length() == 0)
    return false;

  str::const_iterator it = s.begin();
  if (*it == '-')
    return false;

  for (; it != s.end(); ++it)
    if (!isValidFnameChar(*it))
      return false;

  return true;
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

static bool isValidFspath(const str& s)
{
  if (s.length() == 0)
    return false;

  str::const_iterator it = s.begin();
  if (s.size() > 1 && *(s.end() - 1) == '/')
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

// as we might have URLs like https://moep.de in our string we sadly cannot use
// splitStr here ;(
static std::vector<str> splitRedirStr(const str& rs)
{
  std::vector<str> ret;
  if (rs.empty())
    return ret;

  size_t i = rs.find(":");
  if (i == str::npos)
    return ret;

  str code = rs.substr(0, i);
  if (code.empty())
    return ret;
  str url = rs.substr(++i);
  if (url.empty())
    return ret;
  ret.push_back(code);
  ret.push_back(url);
  return ret;
}

// ---------------------------=[ The Real Stuff ]=--------------------------- //

// serverName
bool ConfigParser::_parseTokName(VServerCfg& vcfg)
{
  if ((++_tokIt)->type != TOK_NAME)
    throw std::runtime_error("Wrong token while parsing serverName");

  if (!isValidDomainName(_tokIt->val)) {
    Logger::logCfgErr(_tokIt->line, "serverName has to be valid domain-name!");
    return false;
  }

  vcfg.setServerName(_tokIt->val);

  return true;
}

// route
// TODO checking where?
// TODO  test this!
bool ConfigParser::_parseTokRoute()
{
  if ((++_tokIt)->type != TOK_ROUTE)
    throw std::runtime_error("Wrong token while parsing route");

  _currentRoute.reset();

  if (!isValidRoute(_tokIt->val)) {
    Logger::logCfgErr(_tokIt->line, "Invalid route!");
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
    Logger::logCfgErr(_tokIt->line, "Invalid interface value");
    return false;
  }

  strip(split[0]);
  strip(split[1]);

  if (!isValidDomainName(split[0])) {
    Logger::logCfgErr(_tokIt->line, "invalid iface addr - ip or domain!)");
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
    Logger::logCfgErr(_tokIt->line, "Invalid errorPage!");
    return false;
  }

  strip(split[0]);
  strip(split[1]);

  e_HTTPStatus errCode = WsrvLib::short2HttpStatus(str2u16(split[0]));

  if (!isValidRoute(split[1])) {
    Logger::logCfgErr(_tokIt->line, "Invalid file route for errorPage!");
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
  if ((++_tokIt)->type != TOK_FNAME)
    throw std::runtime_error("Wrong token while parsing index");

  str& val = _tokIt->val;
  strip(val);

  if (!isValidFname(val)) {
    Logger::logCfgErr(_tokIt->line, "'" + val + "' is invalid filename");
    return false;
  }
  _currentRoute.setIndex(val);
  return true;
}

// autoindex
bool ConfigParser::_parseTokBool()
{
  if ((++_tokIt)->type != TOK_BOOL)
    throw std::runtime_error("Wrong token while parsing autoindex");

  str& val = _tokIt->val;
  strip(val);

  if (val == "true" || val == "on")
    _currentRoute.setAutoindex(true);
  else if (val == "false" || val == "off")
    _currentRoute.setAutoindex(false);
  else {
    Logger::logCfgErr(
        _tokIt->line, "Only on/off || true/false allowed as boolean values!");
    return false;
  }
  return true;
}

// methods
// GET POST DELETE
bool ConfigParser::_parseTokMeth()
{
  if ((++_tokIt)->type != TOK_METH)
    throw std::runtime_error("Wrong token while parsing methods");

  std::vector<str> meths = splitStrWhite(_tokIt->val);

  _currentRoute.clearMethods();

  for (std::vector<str>::iterator it = meths.begin(); it != meths.end(); ++it) {
    e_Method m = str2meth(*it);
    if (m == M_UNKNOWN) {
      Logger::logCfgErr(_tokIt->line, "Method unknown!");
      return false;
    }
    _currentRoute.addMethod(m);
  }

  return true;
}

// redirect
// Ex: 302:/moep/miep
// Ex: 302:https://atat.de
//
// Do like nginx: don't check anything in here. We will send the Location to
// client no matter what and it will have to deal with it.
bool ConfigParser::_parseTokRedir()
{
  if ((++_tokIt)->type != TOK_REDIR)
    throw std::runtime_error("Wrong token while parsing redir");

  str& redir = _tokIt->val;
  strip(redir);

  std::vector<str> sp = splitRedirStr(redir);

  if (sp.size() != 2) {
    Logger::logCfgErr(_tokIt->line, "Malformed redir string!");
    return false;
  }

  e_HTTPStatus code = WsrvLib::str2HttpStatus(sp[0]);
  if (code == HTTP_0 || code < HTTP_300 || code > HTTP_308) {
    Logger::logCfgErr(_tokIt->line, "Invalid status code in redir direc!");
    return false;
  }

  _currentRoute.setRedir(code, sp[1]);

  return true;
}

// cgi
// Ex: cgi py:/usr/bin/env python!
bool ConfigParser::_parseTokCgi()
{
  if ((++_tokIt)->type != TOK_CGI)
    throw std::runtime_error("Wrong token while parsing cgi");

  std::vector<str> sp = splitString(_tokIt->val, ":");

  if (sp.size() != 2) {
    Logger::logCfgErr(_tokIt->line, "Invalid cgi value!");
    return false;
  }
  sp[0] = strip(sp[0]);
  sp[1] = strip(sp[1]);

  // FIXME: any error checks here?
  _currentRoute.addCgi(sp[0], sp[1]);

  return true;
}

// ------------------------=[ Mixed Scope Parsing ]=------------------------- //

// maxBodySize
bool ConfigParser::_parseTokBytes(VServerCfg& vcfg)
{
  if ((++_tokIt)->type != TOK_BYTES)
    throw std::runtime_error("Wrong token while parsing maxBodySize");

  str& val = _tokIt->val;
  strip(val);
  if (!isNumStr(val))
    return false;

  u32 bytes = std::atoi(val.c_str());
  if (bytes > MAX_BYTES) {
    Logger::logCfgErr(
        _tokIt->line, "maxBodySize >" + int2str(MAX_BYTES) + " not allowed!");
    return false;
  }

  if (_scope.top() == S_SERVER)
    vcfg.setMaxBodySize(bytes);
  else
    _currentRoute.setMaxBodySize(bytes);

  return true;
}

// root, upload
bool ConfigParser::_parseTokFspath(VServerCfg& vcfg)
{
  e_Direcs dir = _tokIt->direc;

  if ((++_tokIt)->type != TOK_FSPATH)
    throw std::runtime_error("Wrong token while parsing fspath");

  str& val = _tokIt->val;
  val      = strip(val);

  if (!isValidFspath(val)) {
    Logger::logCfgErr(_tokIt->line, "invalid fspath: " + val);
    return false;
  }

  switch (dir) {
    case DIR_ROOT: {
      if (_scope.top() == S_SERVER)
        vcfg.setRoot(val);
      else
        _currentRoute.setRoot(val);
      break;
    }
    case DIR_UPLOAD:
      _currentRoute.setUpload(val);
      break;
    default:
      throw std::runtime_error("Ehm.. wrong direc in _parseTokFspath");
  }

  return true;
}
