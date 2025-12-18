/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:35:29 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/18 21:38:23 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

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

// max num of clients.
#define MAX_CLIENTS 1000

// just for explicitness sake define these 2 as macros
enum { REQ_READ = 0, REQ_WRITE = 1, REQ_ERR = -1, REQ_NTERM = -2 };

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
		std::map<int, Server *> _clientFdServerMap;
		size_t									_numOfServers;
		uint16_t								_numOfClients;

		Epoll _epoll;

		void _setupServers();
		void _setupSingleServer(Server& srv);
		void _setupEpoll();
		void _shutdownAllServers();

		// utils
		bool		_isServerFd(int fd) const;
		Server *_getServerByFd(int fd);
		Server *_getServerByClientFd(int fd);
		void		_addClientToClientFdServerMap(int fd, Server *srv);
		void		_initDefaultCfg();
		void		_initDefaultCfg2();

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
