/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser_Tokenize.cpp                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/07 20:08:59 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/08 12:14:11 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include "Logger.hpp"
#include "utils.hpp"

static void skipSpace(str::iterator& it);

void ConfigParser::_tokenize()
{
  if (this->bad())
    throw std::runtime_error("bad cfgfile!");

  _scope.push(S_GLOBAL);

  t_Token lastTok;
  lastTok.type = TOK_NULL;

  str    line;
  size_t lineCnt = 0;
  while (std::getline(_cfgfile, line)) {
    ++lineCnt;
    // std::cout << "line " << lineCnt << ": " << currentLine << std::endl;
    if (line.empty())
      continue;
    str::iterator it  = line.begin();
    str::iterator end = line.end();
    skipSpace(it);
    if (*it == '#')
      continue;
    while (it != end) {
      t_Token newtok;

      try {
        newtok = _readTok(it, end, lastTok);
      } catch (const std::exception& e) {
        throw std::runtime_error("Error parsing token in config-line " +
            int2str(lineCnt) + ": " + e.what());
      }
      _tokens.push_back(newtok);
      lastTok = newtok;
      skipSpace(it);
    }
  }
}

void ConfigParser::_readTokVal(
    t_Token& tok, str::iterator& it, const str::iterator& end)
{
  str::iterator itTmp = it;

  switch (tok.type) {
    case TOK_NULL:
      throw std::runtime_error("Got TOK_NULL where i shouldn't");
    case TOK_BSTART:
      if (*it != '{')
        throw std::runtime_error("Expected '{' got " + int2str(*it));
      tok.val = *it;
      ++it;
      break;
    case TOK_BEND:
      if (*it != '}')
        throw std::runtime_error("Expected '}' got " + int2str(*it));
      tok.val = *it;
      ++it;
      break;
    case TOK_BANG:
      if (*it != BANG)
        throw std::runtime_error("Expected '!' got " + char2str(*it));
      tok.val = *it;
      ++it;
      break;
    case TOK_DIREC: {
      while (isalpha(*itTmp) && itTmp != end)
        ++itTmp;
      str word(it, itTmp);
      Logger::log_bug("word = " + word);
      e_Direcs dir = _str2direc(word);
      if (dir == DIR_INVALID)
        throw std::runtime_error("Unknown Direc");
      tok.direc = dir;
      it        = itTmp;
      break;
    }
    case TOK_BYTES:
    case TOK_STATUS:
    case TOK_FSPATH:
    case TOK_ROUTE:
    case TOK_FNAME:
    case TOK_NAME:
    case TOK_FROUTE:
    case TOK_BOOL:
      while (itTmp != end && *itTmp != BANG && !isspace(*itTmp))
        ++itTmp;
      if (itTmp == end)
        throw std::runtime_error("No Bang!");
      tok.val.assign(it, itTmp);
      it = itTmp;
      break;
    case TOK_METH:
    case TOK_IFACE:
    case TOK_REDIR:
    case TOK_CGI: {
      str::iterator itTmp = it;
      while (itTmp != end && *itTmp != '!')
        ++itTmp;
      if (itTmp == end)
        throw std::runtime_error("No Bang!");
      tok.val.assign(it, itTmp);
      it = itTmp;
    }
  }
}

// Rulez:
//
//   - after TOK_BANG, TOK_BSTART and TOK_BEND there will always be TOK_DIREC or
//   nothing
ConfigParser::t_Token ConfigParser::_readTok(
    str::iterator& it, const str::iterator& end, const t_Token& lasttok)
{
  t_Token newtok;

  switch (lasttok.type) {
    case TOK_BANG:
    case TOK_BSTART:
    case TOK_BEND:
    case TOK_NULL:
      if (isalpha(*it))
        newtok.type = TOK_DIREC;
      else
        newtok.type = TOK_BEND;
      break;
    case TOK_IFACE:
    case TOK_BOOL:
    case TOK_REDIR:
    case TOK_CGI:
    case TOK_METH:
    case TOK_FSPATH:
    case TOK_BYTES:
    case TOK_NAME:
    case TOK_FROUTE:
    case TOK_FNAME:
      newtok.type = TOK_BANG;
      break;
    case TOK_ROUTE:
      newtok.type = TOK_BSTART;
      break;
    case TOK_STATUS:
      newtok.type = TOK_FSPATH; // QUESTION: not sure about that
      break;
    case TOK_DIREC:
      newtok.type = _nextTokFromDirec(lasttok.direc);
  }

  try {
    _readTokVal(newtok, it, end);
  } catch (const std::exception& e) {
    throw;
  }

  // WARN: in principle this is not completely clean as the scope can only be
  // switched after having read the TOK_BSTART. But we'll see.
  if (_scope.top() != S_GLOBAL && newtok.type == TOK_DIREC &&
      newtok.direc == DIR_SERVER)
    throw std::runtime_error("Wrong scope for server");
  if (newtok.type == TOK_DIREC) {
    if (newtok.direc == DIR_SERVER)
      _scope.push(S_SERVER);
    if (newtok.direc == DIR_ROUTE)
      _scope.push(S_ROUTE);
  }
  if (newtok.type == TOK_BEND)
    _scope.pop();

  return newtok;
}

static void skipSpace(str::iterator& it)
{
  while (std::isspace(*it))
    ++it;
}

ConfigParser::e_Direcs ConfigParser::_str2direc(const str& s)
{
  if (_knownDirectives.find(s) == _knownDirectives.end())
    return DIR_INVALID;

  if (s == "server")
    return DIR_SERVER;
  if (s == "serverName")
    return DIR_SERVERNAME;
  if (s == "route")
    return DIR_ROUTE;
  if (s == "listen")
    return DIR_LISTEN;
  if (s == "maxBodySize")
    return DIR_MAXBODYSIZE;
  if (s == "errorPage")
    return DIR_ERRORPAGE;
  if (s == "defaultFile")
    return DIR_DEFAULTFILE;
  if (s == "autoindex")
    return DIR_AUTOINDEX;
  if (s == "methods")
    return DIR_METHODS;
  if (s == "root")
    return DIR_ROOT;
  if (s == "upload")
    return DIR_UPLOAD;
  if (s == "redirect")
    return DIR_REDIRECT;
  if (s == "index")
    return DIR_INDEX;
  return DIR_CGI;
}

ConfigParser::e_TokType ConfigParser::_nextTokFromDirec(e_Direcs direc)
{
  switch (direc) {
    case DIR_SERVER:
      return TOK_BSTART;
    case DIR_SERVERNAME:
      return TOK_NAME;
    case DIR_ROUTE:
      return TOK_ROUTE;
    case DIR_LISTEN:
      return TOK_IFACE;
    case DIR_MAXBODYSIZE:
      return TOK_BYTES;
    case DIR_ERRORPAGE:
      return TOK_STATUS;
    case DIR_DEFAULTFILE:
      return TOK_FNAME;
    case DIR_AUTOINDEX:
      return TOK_BOOL;
    case DIR_METHODS:
      return TOK_METH;
    case DIR_ROOT:
      return TOK_FSPATH;
    case DIR_UPLOAD:
      return TOK_FSPATH; // arbitrary path possible here, also /tmp!
    case DIR_REDIRECT:
      return TOK_REDIR;
    case DIR_INDEX:
      return TOK_FNAME;
    case DIR_CGI:
      return TOK_CGI;
    case DIR_INVALID:
      return TOK_NULL;
  }
}
