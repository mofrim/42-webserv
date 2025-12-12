/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:50:36 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/08 15:42:47 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "ServerCfg.hpp"

#include <stdexcept>

// the Server class
//
// can only be constructed using a ServerCfg.
class Server {
	private:
		Server();
		uint16_t					 _port;
		in_addr_t					 _host;
		std::string				 _server_name;
		std::string				 _root;
		struct sockaddr_in _server_addr;
		int								 _listen_fd;
		int								 _epoll_fd;

		void _setupSocket();
		void _setupEpoll();

	public:
		// OCF
		Server(const ServerCfg& srvcfg);
		Server(const Server& other);
		Server& operator=(const Server& other);
		~Server();

		std::string getServerName() const;

		void init();
		void run();

		// TODO: think about the whole exception thing! Maybe there should be
		// some base-class called like this one and the special exceptions for every
		// thing that can fail (~ sounds like a lot of work to me :/ )
		class ServerInitException: public std::runtime_error {
			public:
				ServerInitException(const std::string& msg);
		};

		class ServerRunException: public std::runtime_error {
			public:
				ServerRunException(const std::string& msg);
		};
};
#endif
