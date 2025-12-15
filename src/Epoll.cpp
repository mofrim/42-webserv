/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 23:12:17 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/15 00:41:38 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Epoll.hpp"
#include "Logger.hpp"

#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////
/// OCF

Epoll::Epoll() {}

Epoll::Epoll(const Epoll& other) { (void)other; }

Epoll& Epoll::operator=(const Epoll& other)
{
	(void)other;
	return (*this);
}

Epoll::~Epoll() {}

////////////////////////////////////////////////////////////////////////////////
/// methods

void Epoll::setup(const std::vector<Server>& servers,
		const size_t&														 numOfServers)
{
	int listen_fd;

	_ev.resize(numOfServers);

	// arg for epoll_create only has to be a positive number, so...
	_epoll_fd = epoll_create(42);
	if (_epoll_fd == -1)
		throw(EpollException("epoll_create failed"));

	for (size_t i = 0; i < numOfServers; i++) {
		listen_fd			 = servers[i].getListenFd();
		_ev[i].events	 = EPOLLIN;
		_ev[i].data.fd = listen_fd;

		if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, listen_fd, &_ev[i]) == -1) {
			close(_epoll_fd);
			throw(EpollException("epoll_ctl failed"));
		}
	}
}

// a simple wrapper for our one and only epoll_wait call
int Epoll::wait()
{
	Logger::log_msg("calling epoll_wait...");
	return (epoll_wait(_epoll_fd, _events, MAX_EVENTS, 500));
}

// Add a client's fd to epoll interest list
void Epoll::addClient(int cfd)
{
	struct epoll_event client_ev;
	client_ev.events	= EPOLLIN;
	client_ev.data.fd = cfd;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, cfd, &client_ev) == -1) {
		Logger::log_err("epoll_ctl failed");
		close(cfd);
		throw(EpollException("adding client to epoll failed"));
	}
}

// First delete a client's fd from the epoll interest list and then close it.
// the other way round epoll_ctl fails.
void Epoll::removeClient(int cfd)
{
	int epoll_del_return = epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, cfd, 0);
	int close_return		 = close(cfd);

	if (epoll_del_return == -1 || close_return == -1)
		throw(EpollException("removing client from epoll failed"));
}

// wrapper for closing the epoll fd.
void Epoll::closeEpollFd()
{
	Logger::log_warn("closing epoll fd");
	close(_epoll_fd);
}

int Epoll::getEpollFd() const { return (_epoll_fd); }

int Epoll::getEventFd(int event_idx) const
{
	return (_events[event_idx].data.fd);
}

////////////////////////////////////////////////////////////////////////////////
/// Exceptions

Epoll::EpollException::EpollException(const std::string& msg):
	std::runtime_error("EpollException: " + msg)
{}
