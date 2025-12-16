/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:35:29 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/16 11:47:55 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "ConManager.hpp"
#include "Epoll.hpp"
#include "Server.hpp"

#include <map>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>

// default port for the default server
#define DEFAULT_PORT 4284

// default server name
#define DEFAULT_SRV_NAME "localhost"

// NOTE: Naming convention for private class-members / methods: underscore at
// the beginning. In later use this enables us to omit the `this->`. This saves
// a lot of typing while being clear enough as we are still hand-writing our
// code here ;)

class Webserv {
	private:
		bool										_defaultCfg;
		bool										_shutdown_server;
		std::vector<Server>			_servers;
		std::map<int, Server *> _serverFdMap;
		size_t									_numOfServers;

		Epoll			 _epoll;
		ConManager _con;

		void _setupServers();
		void _setupSingleServer(Server& srv);
		void _initDefaultCfg();
		void _initDefaultCfg2();
		void _setupEpoll();
		bool _isServerFd(int fd) const;

		// we do not use them, so keep em private
		Webserv(const Webserv& other);
		Webserv& operator=(const Webserv& other);

	public:
		Webserv();
		~Webserv();

		void run();
		void getServersFromCfg(const std::string& cfgFilename);
		void shutdown();

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
