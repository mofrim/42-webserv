/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:36:43 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/16 13:10:58 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Logger.hpp"
#include "Server.hpp"
#include "Webserv.hpp"

// FIXME: remove cause sleep is not an allowed function!
#include <unistd.h>

// for memset
#include <cstring>
#include <errno.h>
#include <iostream>
#include <utils.hpp>

Webserv::Webserv(): _defaultCfg(true), _shutdown_server(false), _numOfServers(0)
{}

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

// the Idea is: keep the existence of the Config object limited to this method.
// After this method there should be only the vector<Server> which then holds
// all the data. Additionally there can be some global config options which can
// be saved in Webserv class or also in Server class.
void Webserv::getServersFromCfg(const std::string& cfgFilename)
{
	(void)cfgFilename;
	Config cfg;

	try {
		cfg.parseCfgFile(cfgFilename);
	} catch (const std::exception& e) {
		throw(e);
	}

	_numOfServers = cfg.getCfgs().size();

	for (size_t i = 0; i < _numOfServers; i++) {
		_servers.push_back(Server(cfg.getCfgs()[i]));
	}
	_defaultCfg = false;
}

// The main routine for setting up the servers listed in the Config.
//
// In the default case there is only one server with default values (like
// serving from the current dir any index.html or whatever,
// server_name="localhost", port="4284" etc...)
void Webserv::_setupServers()
{
	if (_defaultCfg)
		_initDefaultCfg2();

	for (size_t i = 0; i < _numOfServers; i++) {
		_setupSingleServer(_servers[i]);
	}
}

// here one server is being setup, meaning, the `init()` of a server is called
// which is only responsible for binding a socket to a port and start listening
// on it. The rest of the server initialization is being done in... here too?!
// TODO: handle possible exceptions!!!
void Webserv::_setupSingleServer(Server& srv)
{
	Logger::log_msg("Setting up this server:");
	srv.printCfg();
	srv.init();
	_serverFdMap.insert(std::pair<int, Server *>(srv.getListenFd(), &srv));
}

// NOTE: this _only_ needs to be done for the default config. all non-default
// config wil have gone through parseCfg where all values will be set.
void Webserv::_initDefaultCfg()
{
	Server			dsrv;
	sockaddr_in srv_addr;

	dsrv.setServerName(DEFAULT_SRV_NAME);
	dsrv.setPort(DEFAULT_PORT);
	dsrv.setRoot("./www");

	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family			 = AF_INET;
	srv_addr.sin_addr.s_addr = INADDR_ANY;
	srv_addr.sin_port				 = htons(DEFAULT_PORT);
	dsrv.setServerAddr(srv_addr);
	dsrv.setHost(INADDR_LOOPBACK);

	_numOfServers = 1;
}

void Webserv::_initDefaultCfg2()
{

	Server			dsrv1;
	Server			dsrv2;
	sockaddr_in srv_addr;

	dsrv1.setServerName(DEFAULT_SRV_NAME);
	dsrv1.setPort(DEFAULT_PORT);
	dsrv1.setRoot("./www");

	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family			 = AF_INET;
	srv_addr.sin_addr.s_addr = INADDR_ANY;
	srv_addr.sin_port				 = htons(DEFAULT_PORT);
	dsrv1.setServerAddr(srv_addr);
	dsrv1.setHost(INADDR_LOOPBACK);

	dsrv2.setServerName("0.0.0.0");
	dsrv2.setPort(4285);
	dsrv2.setRoot("./www");

	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family			 = AF_INET;
	srv_addr.sin_addr.s_addr = INADDR_ANY;
	srv_addr.sin_port				 = htons(4285);
	dsrv2.setServerAddr(srv_addr);
	dsrv2.setHost(INADDR_LOOPBACK);

	_servers.push_back(dsrv1);
	_servers.push_back(dsrv2);

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
// EINTR == epoll_wait was interrupted by signal_handler being calles (expl:
// epoll_wait is a syscall and as such atomic. so if any interupt is being
// called the syscall exits)
void Webserv::run()
{
	_setupServers();
	_epoll.setup(_servers, _numOfServers);

	// the main loop
	while (_shutdown_server == false) {
		int nfds = _epoll.wait();
		if (nfds == -1 && errno != EINTR)
			throw(WebservRunException("epoll_wait failed"));

		_epoll.printEvents(); // debug

		for (int eventIdx = 0; eventIdx < nfds; ++eventIdx) {
			int currentFd = _epoll.getEventFd(eventIdx);

			// 1) new connection
			if (_isServerFd(currentFd)) {
				Server *srv = _getServerByFd(currentFd);
				Client *cli = srv->addClient(currentFd);
				Logger::log_dbg2("got this fd from cli: " + int2str(cli->getFd()));
				_addClientToClientFdServerMap(cli->getFd(), srv);
				_epoll.addClient(cli->getFd());
			}

			// 2) existing connection
			else {
				Server *srv = _getServerByClientFd(currentFd);
				if (srv == NULL)
					throw(WebservRunException("could not find server by fd"));
				if (srv->handleEvent(_epoll.getEvent(eventIdx), currentFd))

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
