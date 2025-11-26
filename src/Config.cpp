/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 11:12:20 by fmaurer           #+#    #+#             */
/*   Updated: 2025/11/26 09:49:52 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "ConfigParser.hpp"
#include "Logger.hpp"

#include <iostream>

//// OCF

// Default constructor initializes Config with only the default server
// available.
Config::Config(): _isDefaultCfg(true)
{
	ServerCfg defaultServer;
	this->_cfgs.push_back(defaultServer);
}

Config::Config(const Config& other):
	_isDefaultCfg(other._isDefaultCfg), _cfgs(other._cfgs)
{}

Config& Config::operator=(const Config& other)
{
	if (this != &other) {
		_isDefaultCfg = other._isDefaultCfg;
		_cfgs					= other._cfgs;
	}
	return (*this);
}

Config::~Config() {}

//// OCF end

void Config::_setIsDefaultCfg(bool state) { _isDefaultCfg = state; }

bool Config::isDefaultCfg() const { return (_isDefaultCfg); }

Config::EmptyCfgVectorException::EmptyCfgVectorException(
		const std::string& msg):
	std::logic_error("EmptyCfgsVectorException: " + msg)
{
	Logger::log_err("Exception was thrown! Shutting down server!");
}

void Config::setCfgs(std::vector<ServerCfg> cfgs)
{
	if (cfgs.size() == 0)
		throw(Config::EmptyCfgVectorException(
				"somehow u managed to pass an empty cfg vector."));
	_cfgs = cfgs;
}

// TODO: implement this...
// Maybe we put another step parser.checkCfg() in here...
void Config::parseCfgFile(const std::string& fname)
{
	ConfigParser parser;

	try {
		parser.openCfg(fname);
		_cfgs = parser.parse();
		_setIsDefaultCfg(false);
	} catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}

std::vector<ServerCfg> Config::getCfgs() const { return (this->_cfgs); }
