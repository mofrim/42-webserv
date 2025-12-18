/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv_Utils.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 11:54:27 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/18 17:16:14 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

// for memset
#include <cstring>

bool Webserv::_isServerFd(int fd) const
{
	return (_serverFdMap.find(fd) != _serverFdMap.end());
}

Server *Webserv::_getServerByFd(int fd)
{
	std::map<int, Server *>::iterator it;
	it = _serverFdMap.find(fd);
	if (it == _serverFdMap.end())
		return (NULL);
	return (it->second);
}

Server *Webserv::_getServerByClientFd(int fd)
{
	std::map<int, Server *>::iterator it;
	it = _clientFdServerMap.find(fd);
	if (it == _clientFdServerMap.end())
		return (NULL);
	return (it->second);
}

void Webserv::_addClientToClientFdServerMap(int fd, Server *srv)
{
	_clientFdServerMap.insert(std::pair<int, Server *>(fd, srv));
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

	dsrv1.setServerName("4284");
	dsrv1.setPort(DEFAULT_PORT);
	dsrv1.setRoot("./www");

	memset(&srv_addr, 0, sizeof(srv_addr));
	srv_addr.sin_family			 = AF_INET;
	srv_addr.sin_addr.s_addr = INADDR_ANY;
	srv_addr.sin_port				 = htons(DEFAULT_PORT);
	dsrv1.setServerAddr(srv_addr);
	dsrv1.setHost(INADDR_LOOPBACK);

	dsrv2.setServerName("4285");
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
