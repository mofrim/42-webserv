/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 09:14:47 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/09 17:43:05 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include "Logger.hpp"

ConfigParser::ConfigParser() {}

ConfigParser::ConfigParser(const ConfigParser& other) { (void)other; }

ConfigParser& ConfigParser::operator=(const ConfigParser& other)
{
  (void)other;
  return (*this);
}

ConfigParser::~ConfigParser() {}

ConfigParser::ConfigParser(const str& cfgFname): _bad(false)
{
  _cfgfile.open(cfgFname.c_str(), std::ios_base::in);
}

bool ConfigParser::bad() const { return !_cfgfile.good() || _bad; }

const std::vector<VServerCfg>& ConfigParser::getCfgs() const { return _vcfgs; }

void ConfigParser::parse()
{
  try {
    _tokenize();

    if (LOGLEVEL >= LOG_DEBUG || g_WsrvTesting)
      dbgPrintTokens();

    _processTokens();
  } catch (const std::runtime_error& e) {
    throw;
  }
}

// ------------------------=[ KnownDirectives Set  ]=------------------------ //

// 14 directives i shall support
std::set<str> ConfigParser::_initKnownDirectives()
{
  const char *dirNames[] = {
      "server",
      "serverName",
      "route",
      "listen",
      "maxBodySize",
      "errorPage",
      "defaultFile",
      "autoindex",
      "methods",
      "root",
      "upload",
      "redirect",
      "index",
      "cgi"};

  std::set<str> s(dirNames, dirNames + 14);
  return s;
}

const std::set<str> ConfigParser::_knownDirectives = _initKnownDirectives();

std::set<str> ConfigParser::getKnownDirectives() { return _knownDirectives; }
