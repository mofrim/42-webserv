/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 11:11:31 by fmaurer           #+#    #+#             */
/*   Updated: 2025/11/26 09:49:37 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "ServerCfg.hpp"

#include <stdexcept>
#include <vector>

// The Basic Desgin Idea here is:
//
// - If webserv program is called without any cmdline arg we launch the default
// config.
// - If webserv is launched with one cmdline arg we will try and parse the file
// given. If successful the _isDefaultCfg flag will be false.

class Config {
	private:
		bool									 _isDefaultCfg;
		std::vector<ServerCfg> _cfgs;
		void									 _setIsDefaultCfg(bool state);

	public:
		// OCF
		Config();
		Config(const Config& other);
		Config& operator=(const Config& other);
		~Config();

		void									 setCfgs(std::vector<ServerCfg> cfgs);
		std::vector<ServerCfg> getCfgs() const;
		void									 parseCfgFile(const std::string& fname);
		bool									 isDefaultCfg() const;

		class EmptyCfgVectorException: public std::logic_error {
			public:
				EmptyCfgVectorException(const std::string& msg);
		};
};

#endif
