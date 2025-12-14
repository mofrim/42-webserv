/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConManager.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 21:20:37 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/14 22:13:44 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// @class ConManager
//
// This class should be responsible for handling Connections to the servers. In
// Contrast to the Webserv class, which is only responsible for setting up the
// servers, handline epoll and shutting them down again. With this class we want
// to keep Webserv class from getting too big.

#ifndef CONMANAGER_HPP
#define CONMANAGER_HPP

#include "Client.hpp"

#include <set>
#include <stdexcept>
#include <vector>

class ConManager {
	private:
		std::vector<Client> _clients;
		std::set<int>				_clientFds;

		// we certainly won't need these 2 for
		ConManager(const ConManager& other);
		ConManager& operator=(const ConManager& other);

	public:
		ConManager();
		~ConManager();

		int addNewClient(int srv_fd);
		int handleRequest(int client_fd);

		class ClientNotFoundException: public std::runtime_error {
			public:
				ClientNotFoundException(const std::string& msg);
		};
};

#endif
