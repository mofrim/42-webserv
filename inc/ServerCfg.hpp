/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerCfg.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 08:01:29 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/03 13:05:27 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCFG_HPP
#define SERVERCFG_HPP

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

class ServerCfg {
	private:
		uint16_t	_port;
		in_addr_t _host; // QUESTION: this should normally be the ip... idk
										 // yet if / when we will need this
		std::string				 _server_name;
		std::string				 _root;
		struct sockaddr_in _server_addr;
		int								 _listen_fd;

		// std::string						_index;
		// std::vector<Location> _locations;
		// unsigned long					_client_max_body_size;

	public:
		// OCF
		ServerCfg();
		ServerCfg(const ServerCfg& other);
		ServerCfg& operator=(const ServerCfg& other);
		~ServerCfg();

		void setPort(uint16_t port);
		void setHost(in_addr_t host);
		void setServerName(std::string name);
		void setRoot(std::string root);
		void setListenFd(int listen_fd);
		void setServerAddr(sockaddr_in server_addr);
		//
		// void setClientMaxBodySize(unsigned long cli_max_body_size);
		// void setIndex(std::string index);
		// void setLocations(std::vector<Location> locations);

		uint16_t		getPort() const;
		in_addr_t		getHost() const;
		std::string getServerName() const;
		std::string getRoot() const;
		sockaddr_in getServerAddr() const;
		int					getListenFd() const;

		// unsigned long					getClientMaxBodySize();
		// std::string						getIndex();
		// std::vector<Location> getLocations();

		void printCfg() const;
};

#endif
