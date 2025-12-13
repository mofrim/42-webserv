/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:36:43 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/13 08:18:38 by fmaurer          ###   ########.fr       */
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
#include <iostream>
#include <utils.hpp>

Webserv::Webserv(): _shutdown_server(false), _numOfServers(0), _epoll_fd(-1) {}

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

// TODO: what is being done here, what in the main loop. For now calling
// epoll_create is done here and the listening sockets for each server are added
// to the interest-list of epoll.
void Webserv::_setupEpoll()
{
	int listen_fd;

	_ev.resize(_numOfServers);

	// param for epoll_create only has to be a positive number, so...
	_epoll_fd = epoll_create(42);
	if (_epoll_fd == -1)
		throw(WebservInitException("epoll_create failed"));

	for (size_t i = 0; i < _numOfServers; i++) {
		listen_fd			 = _servers[i].getListenFd();
		_ev[i].events	 = EPOLLIN;
		_ev[i].data.fd = listen_fd;

		if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, listen_fd, &_ev[i]) == -1) {
			close(_epoll_fd);
			throw(WebservInitException("epoll_ctl failed"));
		}
	}
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
	_setOfServerFds.insert(srv.getListenFd());
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

Webserv::WebservInitException::WebservInitException(const std::string& msg):
	std::runtime_error("WebservInitException: " + msg)
{}

Webserv::WebservRunException::WebservRunException(const std::string& msg):
	std::runtime_error("WebservRunException: " + msg)
{}

// TODO: where should i start?
void Webserv::run(const Config& cfg)
{
	struct epoll_event events[MAX_EVENTS];

	_cfg = cfg;
	_setupServers();
	_setupEpoll();

	// the main loop
	while (_shutdown_server == false) {

		Logger::log_msg("calling epoll_wait...");

		// TODO: figure out the best timeout value here. For now -1 is okay. but
		// even a small timeout like 10ms might be okay. what are the benefits here?
		// epoll_wait returns the number of ready fds
		int nfds = epoll_wait(_epoll_fd, events, MAX_EVENTS, 500);

		// NOTE: i think checking errno here is okay as the subject only demands not
		// checking it after read write.

		if (nfds == -1 && errno != EINTR) // EINTR == epoll_wait was interrupted by
																			// signal
			throw(WebservRunException("epoll_wait failed"));

		for (int i = 0; i < nfds; ++i) {
			int currentFd = events[i].data.fd;
			if (_setOfServerFds.find(currentFd) != _setOfServerFds.end()) {

				Logger::log_dbg("accepting new conn on fd " + int2str(currentFd));

				// accept new connection
				struct sockaddr_in client_addr;
				socklen_t					 client_addr_len = sizeof(client_addr);
				int								 client_fd			 = accept(currentFd,
						 (struct sockaddr *)&client_addr,
						 &client_addr_len);
				if (client_fd == -1) {
					Logger::log_err("accept failed");
					continue;
				}

				Logger::log_dbg("Client connected from address " +
						inaddrToStr(client_addr.sin_addr));

				// Add client socket to epoll
				struct epoll_event client_ev;
				client_ev.events	= EPOLLIN;
				client_ev.data.fd = client_fd;
				if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client_fd, &client_ev) == -1) {
					Logger::log_err("epoll_ctl failed");
					close(client_fd);
					continue;
				}
			}
			else {
				Logger::log_dbg("reading from socket" + int2str(currentFd));
				char		buffer[1024];
				ssize_t bytes_read = read(currentFd, buffer, sizeof(buffer) - 1);
				if (bytes_read <= 0) {
					if (bytes_read == 0) {
						Logger::log_warn("Client disconnected");
						Logger::log_warn("closing client conn on fd " + int2str(currentFd));
						close(events[i].data.fd);
						epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, currentFd, 0);
					}
					else {
						Logger::log_err("read failed, errno: " + int2str(errno));
						if (_setOfServerFds.find(currentFd) == _setOfServerFds.end()) {
							Logger::log_err(
									"closing client conn on fd " + int2str(currentFd));
							close(events[i].data.fd);
							epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, currentFd, 0);
						}
					}
				}
				else {
					buffer[bytes_read] = '\0';
					std::cout << "Received:\n" << buffer << std::endl;
				}
			}
		}
	}
	Logger::log_warn("closing epoll fd");
	close(_epoll_fd);
}
