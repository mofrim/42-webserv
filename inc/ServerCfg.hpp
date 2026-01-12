/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerCfg.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/25 08:01:29 by fmaurer           #+#    #+#             */
/*   Updated: 2026/01/12 17:47:59 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCFG_HPP
#define SERVERCFG_HPP

#include "Route.hpp"

#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <vector>

// all we need in global context:
//
// - the server_name
// - ip:port pairs of interfaces we are listening on
// - the servers root dir
//   -> QUESTION: should we support having this in global OR route context?
// - default file name (QUESTION: look above)  aka index

// NEXT: bit by bit refactor this until we have what we want for config
// compatibility
class ServerCfg {
	private:
		std::string _server_name;

		// i will make this a
		//
		// 	std::vector< std::pair<uint16_t, std::string> >
		//
		// because during config validation we will only check if strings of IPs
		// match _AND_ if the strings represent valid IPv4 (or even IPv6) adresses
		uint16_t	_port;
		in_addr_t _host;

		std::string _root;

		// what is this????
		struct sockaddr_in _server_addr;

		// TODO: rename this to `default_file`
		std::string _default_file;

		unsigned long _client_max_body_size;

		std::vector<Route> _routes;

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
		void setServerAddr(sockaddr_in server_addr);
		void setDefaultFile(std::string default_file);
		//
		// void setClientMaxBodySize(unsigned long cli_max_body_size);
		// void setIndex(std::string index);
		// void setLocations(std::vector<Location> locations);

		uint16_t		getPort() const;
		in_addr_t		getHost() const;
		std::string getServerName() const;
		std::string getRoot() const;
		sockaddr_in getServerAddr() const;
		std::string getDefaultFile() const;

		// unsigned long					getClientMaxBodySize();
		// std::string						getIndex();
		// std::vector<Location> getLocations();

		void printCfg() const;
};

#endif
