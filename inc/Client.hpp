/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 20:50:12 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/16 12:46:20 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <ctime>

class Client {
	private:
		// TODO: somehow add (which?) info to server client is connected with.
		int			_client_fd;
		clock_t _last_access;

	public:
		Client();
		Client(const Client& other);
		Client& operator=(const Client& other);
		~Client();

		Client(int fd, clock_t la);

		void setFd(int fd);
		int	 getFd() const;

		void		setLastAccess(clock_t t);
		clock_t getLastAccess() const;
};

#endif
