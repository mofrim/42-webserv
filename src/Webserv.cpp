/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:36:43 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/10 08:37:41 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Server.hpp"
#include "Webserv.hpp"

// FIXME: remove cause sleep is not an allowed function!
#include <unistd.h>

// for memset
#include <cstring>
#include <iostream>

Webserv::Webserv(): _shutdown_server(false), _epoll_fd(-1) {}

Webserv::Webserv(const Webserv& other) { (void)other; }

Webserv& Webserv::operator=(const Webserv& other)
{
	(void)other;
	return (*this);
}

Webserv::~Webserv() {}

// TODO: there will be much more to do in here. What?
//
// - close sockets
// - disconnect clients
// - etc....
void Webserv::shutdown()
{
	Logger::log_warn("shutting down webserv...");
	_shutdown_server = true;
}

// NOTE: *THIS* is the main routine of the program!!!
void Webserv::run(const Config& cfg)
{
	_cfg = cfg;
	_setupServers();

	// the main loop
	while (!_shutdown_server) {
		for (std::vector<Server>::iterator srv = _servers.begin();
				srv != _servers.end();
				++srv)
		{
			try {
				srv->run();
			} catch (const Server::ServerRunException& e) {
				std::cout << "e.what(): " << e.what() << std::endl;
			}
		}
		Logger::log_msg("webserv running!");
		sleep(1);
	}
}

// The main routine for setting up the servers listed in the Config.
//
// In the default case there is only one server with default values (like
// serving from the current dir any index.html or whatever,
// server_name="localhost", port="4284" etc...)
void Webserv::_setupServers()
{
	if (_cfg.isDefaultCfg()) {
		_initDefaultCfg();
		_setupSingleServer(_cfg.getCfgs()[0]);
	}
	else
		for (size_t i = 0; i < _cfg.getCfgs().size(); i++) {
			_setupSingleServer(_cfg.getCfgs()[i]);
		}
}

// here one server is being setup, meaning, the `init()` of a server is called
// which is only responsible for binding a socket to a port and start listening
// on it. The rest of the server initialization is being done in... here too?!
void Webserv::_setupSingleServer(const ServerCfg& cfg)
{
	Logger::log_msg("Setting up this server:");
	cfg.printCfg();
	Server srv(cfg);
	srv.init();
	_servers.push_back(srv);
}

// NOTE: this _only_ needs to be done for the default config. all non-default
// config wil have gone through parseCfg where all values will be set.
void Webserv::_initDefaultCfg()
{
	ServerCfg		dcfg = _cfg.getCfgs()[0];
	sockaddr_in srv_addr;

	dcfg.setServerName(DEFAULT_SRV_NAME);
	dcfg.setPort(DEFAULT_PORT);
	dcfg.setRoot("./www");

	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family			 = AF_INET;
	srv_addr.sin_addr.s_addr = INADDR_ANY;
	srv_addr.sin_port				 = htons(DEFAULT_PORT);
	dcfg.setServerAddr(srv_addr);
	dcfg.setHost(INADDR_LOOPBACK);

	_cfg.setDefaultCfg(dcfg);
}
