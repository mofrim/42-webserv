/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 09:14:47 by fmaurer           #+#    #+#             */
/*   Updated: 2025/11/26 10:12:58 by fmaurer          ###   ########.fr       */
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

// TODO: implement
void ConfigParser::openCfg(const std::string& fname) { (void)fname; }

// TODO: implement
std::vector<ServerCfg> ConfigParser::parse()
{
	std::vector<ServerCfg> stub;
	return (stub);
}
