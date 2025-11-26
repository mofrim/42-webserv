/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 08:52:11 by fmaurer           #+#    #+#             */
/*   Updated: 2025/11/26 09:35:41 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include "ServerCfg.hpp"

#include <fstream>
#include <vector>

class ConfigParser {
	private:
		std::ofstream _cfgfile;

	public:
		ConfigParser();
		ConfigParser(const ConfigParser& other);
		ConfigParser& operator=(const ConfigParser& other);
		~ConfigParser();

		void									 openCfg(const std::string& fname);
		std::vector<ServerCfg> parse();
};

#endif
