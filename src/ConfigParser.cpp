/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 09:14:47 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/07 15:15:04 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"

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

bool ConfigParser::bad() const { return _cfgfile.bad() || _bad; }

const std::vector<VServerCfg>& ConfigParser::getCfgs() const { return _vcfgs; }

// TODO: implement
void ConfigParser::parse() { std::vector<VServerCfg> stub; }
