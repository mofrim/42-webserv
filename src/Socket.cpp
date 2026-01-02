/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/28 07:26:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/01/02 08:13:18 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"
#include "utils.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <netdb.h>

// -- OCF --

Socket::Socket(): _hostname(""), _port(0) {}

Socket::Socket(const Socket& other): _hostname(""), _port(0) { (void)other; }

Socket& Socket::operator=(const Socket& other)
{
	(void)other;
	return (*this);
}

Socket::~Socket()
{
	if (_addrllist != NULL) {
		std::cout << "Freeing addrlist" << std::endl;
		freeaddrinfo(_addrllist);
	}
}

Socket::Socket(const std::string& hostname, uint16_t port):
	_hostname(hostname), _port(port)
{
	struct addrinfo	 hints;
	struct addrinfo *result;
	int							 s;

	_addrllist = NULL;

	memset(&hints, 0, sizeof(struct addrinfo));

	hints.ai_canonname = NULL;
	hints.ai_addr			 = NULL;
	hints.ai_next			 = NULL;
	hints.ai_family		 = AF_UNSPEC;
	hints.ai_socktype	 = SOCK_STREAM;
	hints.ai_flags		 = AI_PASSIVE | AI_NUMERICSERV | AI_CANONNAME;

	s = getaddrinfo(hostname.c_str(), int2str(port).c_str(), &hints, &result);

	if (s != 0) {
		throw(AddrInfoException(
				"(" + hostname + ", " + int2str(port) + ") " + gai_strerror(s)));
	}

	_addrllist = result;
}

void Socket::print_addrllist() const
{
	struct addrinfo *ap = _addrllist;
	std::string out("addrinfo for " + _hostname + ":" + int2str(_port) + "\n");
	out += "  ---\n";

	while (ap != NULL) {
		switch (ap->ai_family) {
		case AF_UNSPEC:
			out += "  ai_family: AF_UNSPEC\n";
			break;
		case AF_INET:
			out += "  ai_family: AF_INET\n";
			break;
		case AF_INET6:
			out += "  ai_family: AF_INET6\n";
			break;
		default:
			out += "  ai_family: unknown\n";
		}

		switch (ap->ai_socktype) {
		case SOCK_STREAM:
			out += "  ai_socktype: SOCK_STREAM\n";
			break;
		case SOCK_DGRAM:
			out += "  ai_socktype: SOCK_DGRAM\n";
			break;
		default:
			out += "  ai_socktype: 0\n";
		}

		switch (ap->ai_protocol) {
		case IPPROTO_TCP:
			out += "  ai_protocol: IPPROTO_TCP\n";
			break;
		case IPPROTO_UDP:
			out += "  ai_protocol: IPPROTO_UDP\n";
			break;
		default:
			out += "  ai_protocol: 0\n";
		}

		out += "  ai_addrlen: " + int2str(ap->ai_addrlen) + "\n";

		char ip[INET6_ADDRSTRLEN];
		int	 port = 0;
		if (ap->ai_family == AF_INET) {
			if (inet_ntop(ap->ai_family,
							&((struct sockaddr_in *)ap->ai_addr)->sin_addr, ip,
							INET6_ADDRSTRLEN)
					!= NULL)
				out += "  ai_addr: " + std::string(ip) + ", ";
			else
				out += "  ai_addr: " + std::string(strerror(errno)) + ", ";

			port = ntohs(((struct sockaddr_in *)ap->ai_addr)->sin_port);
			out += "port: " + int2str(port) + "\n";
		}
		else {
			if (inet_ntop(ap->ai_family,
							&((struct sockaddr_in6 *)ap->ai_addr)->sin6_addr, ip,
							INET6_ADDRSTRLEN)
					!= NULL)
				out += "  ai_addr: " + std::string(ip) + ", ";
			else
				out += "  ai_addr: " + std::string(strerror(errno)) + ", ";

			port = ntohs(((struct sockaddr_in6 *)ap->ai_addr)->sin6_port);
			out += "port: " + int2str(port) + "\n";
		}

		if (ap->ai_canonname != NULL)
			out += "  ai_canonname: " + std::string(ap->ai_canonname) + "\n";
		else
			out += "  ai_canonname: null\n";

		out += "  ---\n";
		ap = ap->ai_next;
	}

	std::cout << out << std::endl;
}

Socket::AddrInfoException::AddrInfoException(const std::string& msg):
	std::runtime_error("AddrInfoException: " + msg)
{}
