/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConManager.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/14 21:24:38 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/15 07:55:29 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConManager.hpp"
#include "utils.hpp"

#include <Logger.hpp>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <unistd.h>

// -- OCF --
ConManager::ConManager() {}

ConManager::ConManager(const ConManager& other) { (void)other; }

ConManager& ConManager::operator=(const ConManager& other)
{
	(void)other;
	return (*this);
}

ConManager::~ConManager() {}
// -- OCF end --

// adds a new Client to the list and returns its fd
int ConManager::addNewClient(int srv_fd)
{
	Logger::log_dbg("accepting new conn on fd " + int2str(srv_fd));

	struct sockaddr_in client_addr;
	socklen_t					 client_addr_len = sizeof(client_addr);

	int client_fd =
			accept(srv_fd, (struct sockaddr *)&client_addr, &client_addr_len);
	if (client_fd == -1) {
		Logger::log_err("accept failed");
	}
	Logger::log_dbg(
			"Client connected from address " + inaddrToStr(client_addr.sin_addr));

	_clients.push_back(Client(client_fd, clock()));
	_clientFds.insert(client_fd);

	return (client_fd);
}

int ConManager::handleRequest(int client_fd)
{
	if (_clientFds.find(client_fd) == _clientFds.end())
		throw(ClientNotFoundException("client_fd in handleRequest() not found"));

	Logger::log_dbg("reading from socket" + int2str(client_fd));
	char		buffer[1024];
	ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

	// if there was a read error, or there was nothing to read, return 1 in order
	// to signal deletion from epoll interest list
	if (bytes_read <= 0) {
		if (bytes_read == 0) {
			Logger::log_warn("Client disconnected");
			Logger::log_warn("closing client conn on fd " + int2str(client_fd));
		}
		else
			Logger::log_err(
					"read failed, errno: " + int2str(errno) + strerror(errno));
		return (-1);
	}
	// NEXT:
	// IDEA: maybe it is enough to pass the server cfg of the server the
	// client is connected to here?!?!
	// Biiiiiiig QUESTION: how and where is it elegant to really handle the
	// request!?!?!?!? Is there some max-size for a request??!?!?!?!
	// what if i start handling the request down here but do not have any idead
	// about the server config!?!? this would be complete bullshit!!!!
	buffer[bytes_read] = '\0';
	std::cout << "Received:\n" << buffer << std::endl;
	return (0);
}

ConManager::ClientNotFoundException::ClientNotFoundException(
		const std::string& msg):
	std::runtime_error("ClientNotFoundException: " + msg)
{}
