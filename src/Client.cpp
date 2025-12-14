/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 20:51:06 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/14 21:53:09 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

// -- OCF --
Client::Client(): _client_fd(-1) {}

Client::Client(const Client& other) { (void)other; }

Client::Client(int fd, clock_t la): _client_fd(fd), _last_access(la) {}

Client& Client::operator=(const Client& other)
{
	(void)other;
	return (*this);
}

Client::~Client() {}
// -- OCF end --

void Client::setClientFd(int fd) { _client_fd = fd; }
int	 Client::getClientFd() const { return (_client_fd); }

void		Client::setLastAccess(clock_t t) { _last_access = t; }
clock_t Client::getLastAccess() const { return (_last_access); }
