/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 23:11:35 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/18 21:17:57 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLL_HPP
#define EPOLL_HPP

#include "Server.hpp"

#include <sys/epoll.h>
#include <vector>

// max num of events epoll will manage in one epoll_wait loop
#define MAX_EVENTS 64
#define EPOLL_TIMEOUT_MS 500

class Epoll {
	private:
		int															_epoll_fd;
		int															_nfds;
		std::vector<struct epoll_event> _ev;
		struct epoll_event							_events[MAX_EVENTS];

		std::string _getEventStr(const uint32_t& ev) const;

	public:
		Epoll();
		Epoll(const Epoll& other);
		Epoll& operator=(const Epoll& other);
		~Epoll();

		void setup(const std::vector<Server>& servers, const size_t& numOfServers);
		int	 wait();
		void addClient(int cfd);
		void modifyClient(int cfd, uint32_t events);
		void removeClient(int cfd);
		void closeEpollFd();

		int												getEpollFd() const;
		int												getEventFd(int event_idx) const;
		const struct epoll_event& getEvent(int event_idx) const;

		void printEvents() const;

		class EpollException: public std::runtime_error {
			public:
				EpollException(const std::string& msg);
		};
};

#endif
