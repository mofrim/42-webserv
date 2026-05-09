/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser_Process.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/08 16:14:27 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/10 00:19:40 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include "Logger.hpp"
#include "utils.hpp"

#include <iostream>

// trying to be a little bit memoy efficient here by constructing the VServerCfg
// in the _vcfgs vector and then passing a reference to _parseVServer.
void ConfigParser::_processTokens()
{
  _scope.empty();
  _scope.push(S_GLOBAL);

  _tokIt = _tokens.begin();

  for (; _tokIt != _tokens.end(); ++_tokIt) {
    if (_isMetaToken())
      continue;
    if (_tokIt->type == TOK_DIREC && _tokIt->direc == DIR_SERVER) {
      _vcfgs.push_back(VServerCfg());
      _scope.push(S_SERVER);
      bool success = false;
      try {
        success = _parseVServer(_vcfgs.back());
      } catch (const std::runtime_error& e) {
        throw;
      }
      if (!success) {
        std::cout << "no success" << std::endl;
        _vcfgs.pop_back();
      }
    }
    else
      throw std::runtime_error("Expected server block, but there is none.");
  }
}

// there are two possible outcomes for this function:
//
//  1) something deeply irregular happened while trying to parse a route or
//  server. then we throw an exception which will lead to the whole parsing
//  process failing.
//
//  2) something minor irregular like no serverName or sth. then return false
//  which will lead to the server being popped off the back of _vcfgs.
//
// however... i am not yet sure what the real problematic cases for complete
// failure would be.
//
// But i'd pefer to be strict here, and not too permissive in order to not
// having to handle too many default value setting. the user is expected to
// provide a functioning config. only if no config is specified or all config
// parsing fails we use the default cfg.
bool ConfigParser::_parseVServer(VServerCfg& vcfg)
{
  if (_tokIt->type != TOK_DIREC ||
      (_tokIt->type == TOK_DIREC && _tokIt->direc != DIR_SERVER))
    throw std::runtime_error("Cannot parse VServer without DIR_SERVER!");

  // advance past server direc and BSTART
  _tokIt += 2;

  bool success = true;
  // the main loop. parsing the vsrv cfg as long as we hit the next '}' in the
  // server-scope or reach the end of _tokens where a TOK_NULL waits.
  while (!((_scope.top() == S_SERVER && _tokIt->type == TOK_BEND) ||
      _tokIt->type == TOK_NULL))
  {
    _skipFooTokens();

    if (_tokIt->type == TOK_BEND) {
      Logger::log_warn("cfg line " + int2str(_tokIt->line),
          "Empty server block not allowed");
      return false;
    }

    if (_tokIt->type != TOK_DIREC)
      throw std::runtime_error("Sth is really wrong here: Not a direc token!");

    Logger::log_bug("Parsing direc: " + _direc2str(_tokIt->direc));

    try {
      if (_scope.top() == S_SERVER)
        success = _parseServerDirec(vcfg);
      else
        success = _parseRouteDirec(vcfg);
    } catch (const std::runtime_error& e) {
      throw;
    }

    // FIXME: should not fail when only route fails?! am i right?!? ah, okay i
    // wanted to be strict here.
    if (!success)
      return false;

    if (!_isMetaToken())
      throw std::runtime_error("Expected meta-token got sth else here!");

    // ATTENTION: don't forget to push S_ROUTE to _scope if we encounter it in
    // _parseServerDirec!
    _skipFooTokens();
    if (_tokIt->type == TOK_BEND) {

      // FIXME: what happens if route is not complete?!?!
      // when is this the case? when will i have to discard a route due to
      // misconfiguration?
      if (_scope.top() == S_ROUTE && success) {
        vcfg.addRoute(_currentRoute);
        _currentRoute.reset();
      }

      _scope.pop();
      ++_tokIt;
    }
  }
  return true;
}

// ----------------=[ Parsing dispatchers for both scopes ]=---------------- //

// The general idea goes like that: when we reach one of the dispatcher
// functions _tokIt will point to a direc (if not sth bad has hapenned ->
// throw). Then we dispatch to the individual parsing functions which itself
// advance _tokIt one token to the directives value. This value is parsed and
// success is returned. Then, to skip the recently parsed token, **the
// dispatcher functions are responsible for advancing _tokIt! **

// parse a server-block directive.
bool ConfigParser::_parseServerDirec(VServerCfg& vcfg)
{
  bool success = true;
  switch (_tokIt->direc) {
    case DIR_SERVERNAME:
      success = _parseTokName(vcfg); // DONE:
      break;
    case DIR_ROUTE:
      success = _parseTokRoute(); // DONE:
      break;
    case DIR_LISTEN:
      success = _parseTokIface(vcfg); // DONE:
      break;
    case DIR_MAXBODYSIZE:
      success = _parseTokBytes(vcfg); // TODO:
      break;
    case DIR_ERRORPAGE:
      success = _parseTokError(vcfg); // DONE:
      break;
    case DIR_ROOT:
      success = _parseTokFspath(vcfg); // TODO:
      break;
    default:
      return false;
  }
  ++_tokIt;
  return success;
}

bool ConfigParser::_parseRouteDirec(VServerCfg& vcfg)
{
  bool success = true;
  switch (_tokIt->direc) {

      // ATTENTION: for those 2 i have to check ROOT || UPLOAD
    case DIR_ROOT:
      success = _parseTokFspath(vcfg);
      break;
    case DIR_UPLOAD:
      success = _parseTokFspath(vcfg);
      break;

    case DIR_MAXBODYSIZE:
      success = _parseTokBytes(vcfg);
      break;
    case DIR_ERRORPAGE:
      success = _parseTokError(vcfg);
      break;
    case DIR_DEFAULTFILE:
      success = _parseTokFname();
      break;
    case DIR_AUTOINDEX:
      success = _parseTokBool();
      break;
    case DIR_METHODS:
      success = _parseTokMeth();
      break;
    case DIR_REDIRECT:
      success = _parseTokRedir();
      break;
    case DIR_INDEX:
      success = _parseTokFname();
      break;
    case DIR_CGI:
      success = _parseTokCgi();
      break;
    default:
      return false;
  }
  ++_tokIt;
  return success;
}

// deliberately not skipping TOK_BEND here because i need to handle this.
void ConfigParser::_skipFooTokens()
{
  while (_tokIt->type == TOK_BANG || _tokIt->type == TOK_BSTART)
    ++_tokIt;
}

bool ConfigParser::_isMetaToken() const
{
  e_TokType t = _tokIt->type;
  return t == TOK_BEND || t == TOK_BANG || t == TOK_BSTART || t == TOK_NULL;
}
