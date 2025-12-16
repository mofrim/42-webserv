/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 12:50:36 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/16 13:02:12 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "ServerCfg.hpp"

#include <list>
#include <map>
#include <stdexcept>

// the Server class
//
// can only be constructed using a ServerCfg.
class Server {
	private:
		uint16_t								_port;
		in_addr_t								_host;
		std::string							_server_name;
		std::string							_root;
		struct sockaddr_in			_server_addr;
		int											_listen_fd;
		std::list<Client>				_clients;
		std::map<int, Client *> _clientFdMap;

		void _setupSocket();

	public:
		// OCF
		Server();
		Server(const ServerCfg& srvcfg);
		Server(const Server& other);
		Server& operator=(const Server& other);
		~Server();

		void		init();
		Client *addClient(int fd);
		int			handleEvent(const struct epoll_event& ev, int client_fd);

		// utils, getters setters
		uint16_t		getPort() const;
		in_addr_t		getHost() const;
		std::string getServerName() const;
		std::string getRoot() const;
		sockaddr_in getServerAddr() const;
		int					getListenFd() const;
		void				setPort(uint16_t port);
		void				setHost(in_addr_t host);
		void				setServerName(std::string name);
		void				setRoot(std::string root);
		void				setListenFd(int listen_fd);
		void				setServerAddr(sockaddr_in server_addr);
		void				printCfg() const;

		// TODO: think about the whole exception thing! Maybe there should be
		// some base-class called like this one and the special exceptions for every
		// thing that can fail (~ sounds like a lot of work to me :/ )
		class ServerInitException: public std::runtime_error {
			public:
				ServerInitException(const std::string& msg);
		};

		class ServerException: public std::runtime_error {
			public:
				ServerException(const std::string& msg);
		};
};
#endif
