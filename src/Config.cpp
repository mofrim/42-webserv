/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 11:12:20 by fmaurer           #+#    #+#             */
/*   Updated: 2025/11/25 15:21:11 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Logger.hpp"

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
		this->_isDefaultCfg = other._isDefaultCfg;
		this->_cfgs					= other._cfgs;
	}
	return (*this);
}

Config::~Config() {}

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
	this->_cfgs = cfgs;
}

// TODO: implement this
void Config::parseCfgFile(const std::string& fname) { (void)fname; }

std::vector<ServerCfg> Config::getCfgs() const { return (this->_cfgs); }
