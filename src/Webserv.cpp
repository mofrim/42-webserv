/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:36:43 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/15 07:53:12 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Server.hpp"
#include "Webserv.hpp"

// FIXME: remove cause sleep is not an allowed function!
#include <unistd.h>

// for memset
#include <cstring>
#include <errno.h>
#include <utils.hpp>

Webserv::Webserv(): _shutdown_server(false), _numOfServers(0) {}

Webserv::Webserv(const Webserv& other) { (void)other; }

Webserv& Webserv::operator=(const Webserv& other)
{
	(void)other;
	return (*this);
}

// so far only closing all server sockets here
Webserv::~Webserv()
{
	for (size_t i = 0; i < _numOfServers; i++)
		close(_servers[i].getListenFd());
}

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

// The main routine for setting up the servers listed in the Config.
//
// In the default case there is only one server with default values (like
// serving from the current dir any index.html or whatever,
// server_name="localhost", port="4284" etc...)
void Webserv::_setupServers()
{
	if (_cfg.isDefaultCfg())
		_initDefaultCfg2();

	_numOfServers = _cfg.getCfgs().size();

	for (size_t i = 0; i < _numOfServers; i++) {
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
	_serverFds.insert(srv.getListenFd());
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
	_numOfServers = 1;
}

void Webserv::_initDefaultCfg2()
{
	ServerCfg		dcfg	= _cfg.getCfgs()[0];
	ServerCfg		dcfg2 = _cfg.getCfgs()[0];
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

	dcfg2.setServerName("0.0.0.0");
	dcfg2.setPort(4285);
	dcfg2.setRoot("./www");

	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family			 = AF_INET;
	srv_addr.sin_addr.s_addr = INADDR_ANY;
	srv_addr.sin_port				 = htons(4285);
	dcfg2.setServerAddr(srv_addr);
	dcfg2.setHost(INADDR_LOOPBACK);

	std::vector<ServerCfg> newcfgs;
	newcfgs.push_back(dcfg);
	newcfgs.push_back(dcfg2);

	_cfg.setCfgs(newcfgs);
	_numOfServers = 2;
}

// TODO: figure out the best timeout for epoll_wait. For now -1 is okay. but
// even a small timeout like 10ms might be okay. what are the benefits here?
//
// NOTE: i think checking errno here is okay as the subject only demands not
// checking it after read write.
//
// the logic here is simple: if we are getting an I/O event on one of our
// servers listening sockets (aka ports) this is a connection request ->
// add a new client!
//
// EINTR == epoll_wait was interrupted by
void Webserv::run(const Config& cfg)
{
	_cfg = cfg;
	_setupServers();
	_epoll.setup(_servers, _numOfServers);

	// the main loop
	while (_shutdown_server == false) {
		int nfds = _epoll.wait();
		if (nfds == -1 && errno != EINTR)
			throw(WebservRunException("epoll_wait failed"));
		for (int i = 0; i < nfds; ++i) {
			int currentFd = _epoll.getEventFd(i);
			if (_serverFds.find(currentFd) != _serverFds.end()) {
				int client_fd = _con.addNewClient(currentFd);
				_epoll.addClient(client_fd);
			}
			else {
				int requestStatus = _con.handleRequest(currentFd);
				if (requestStatus == -1)
					_epoll.removeClient(currentFd);
			}
		}
	}
	_epoll.closeEpollFd();
}

////////////////////////////////////////////////////////////////////////////////
/// Exceptions

Webserv::WebservInitException::WebservInitException(const std::string& msg):
	std::runtime_error("WebservInitException: " + msg)
{}

Webserv::WebservRunException::WebservRunException(const std::string& msg):
	std::runtime_error("WebservRunException: " + msg)
{}
