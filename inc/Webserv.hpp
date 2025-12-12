/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:35:29 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/12 18:03:18 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "Config.hpp"
#include "Server.hpp"

#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

// default port for the default server
#define DEFAULT_PORT 4284

// default server name
#define DEFAULT_SRV_NAME "localhost"

// max num of events epoll will manage to in one epoll_wait loop
#define MAX_EVENTS 64

// NOTE: Naming convention for private class-members / methods: underscore at
// the beginning. In later use this enables us to omit the `this->`. This saves
// a lot of typing while being clear enough as we are still hand-writing our
// code here ;)

class Webserv {
	private:
		bool								_shutdown_server;
		Config							_cfg;
		std::vector<Server> _servers;
		size_t							_numOfServers;

		int															_epoll_fd;
		std::vector<struct epoll_event> _ev;

		void _setupServers();
		void _setupSingleServer(const ServerCfg& cfg);
		void _initDefaultCfg();
		void _setupEpoll();

		// we do not use them, so keep em private
		Webserv(const Webserv& other);
		Webserv& operator=(const Webserv& other);

	public:
		Webserv();
		~Webserv();

		void shutdown();
		void run(const Config& cfg);

		class WebservInitException: public std::runtime_error {
			public:
				WebservInitException(const std::string& msg);
		};

		class WebservRunException: public std::runtime_error {
			public:
				WebservRunException(const std::string& msg);
		};
};

#endif
