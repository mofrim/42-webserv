/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:35:29 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/03 15:51:49 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "Config.hpp"
#include "Server.hpp"

#include <netinet/in.h>
#include <sys/socket.h>

#define DEFAULT_PORT 4284
#define DEFAULT_SRV_NAME "localhost"

// NOTE: Naming convention for private class-members / methods: underscore at
// the beginning. In later use this enables us to omit the `this->`. This saves
// a lot of typing while being clear enough as we are still hand-writing our
// code here ;)

class Webserv {
	private:
		bool									_shutdown_server;
		Config								_cfg;
		std::vector<Server *> _servers;
		void									_setupServers();
		void									_setupOneServer(const ServerCfg& cfg);
		void									_initDefaultCfg();

	public:
		// OCF
		Webserv();
		Webserv(const Webserv& other);
		Webserv& operator=(const Webserv& other);
		~Webserv();

		void shutdown();
		void run(const Config& cfg);
};

#endif
