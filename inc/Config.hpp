/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 11:11:31 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/15 23:29:23 by fmaurer          ###   ########.fr       */
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
		std::vector<ServerCfg> _cfgs;

	public:
		Config();
		Config(const Config& other);
		Config& operator=(const Config& other);
		~Config();

		void													setCfgs(std::vector<ServerCfg> cfgs);
		const std::vector<ServerCfg>& getCfgs() const;
		void													parseCfgFile(const std::string& fname);

		class EmptyCfgVectorException: public std::logic_error {
			public:
				EmptyCfgVectorException(const std::string& msg);
		};
};

#endif
