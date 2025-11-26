/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:36:43 by fmaurer           #+#    #+#             */
/*   Updated: 2025/11/26 10:46:59 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Webserv.hpp"

// FIXME: remove cause sleep is not an allowed function!
#include <unistd.h>

// for memset
#include <cstring>

Webserv::Webserv(): _shutdown_server(false) {}

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
		_setupOneServer(_cfg.getCfgs()[0]);
	}
}

// NEXT: implement a first socket setup for only the default Server
void Webserv::_setupOneServer(const ServerCfg& cfg) { cfg.printCfg(); }

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
