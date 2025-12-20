/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 20:50:12 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/19 18:19:22 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <ctime>
#include <netinet/in.h>
#include <string>

class Client {
	private:
		int					_client_fd;
		std::string _hostname;
		in_port_t		_port;

		clock_t _last_access;

	public:
		Client();
		Client(const Client& other);
		Client& operator=(const Client& other);
		~Client();

		Client(int fd, const std::string& hostname, in_port_t port);

		void setFd(int fd);
		int	 getFd() const;

		void		setLastAccess(clock_t t);
		clock_t getLastAccess() const;
};

#endif
