/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv_Utils.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/16 11:54:27 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/16 12:55:14 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

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
	it = _ClientFdServerMap.find(fd);
	if (it == _ClientFdServerMap.end())
		return (NULL);
	return (it->second);
}

void Webserv::_addClientToClientFdServerMap(int fd, Server *srv)
{
	_ClientFdServerMap.insert(std::pair<int, Server *>(fd, srv));
}
