/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 19:13:35 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/18 21:44:12 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "RequestHandler.hpp"
#include "Server.hpp"
#include "Webserv.hpp"
#include "utils.hpp"

#include <errno.h>
#include <iostream>
#include <string.h>
#include <unistd.h>

// -- OCF --

RequestHandler::RequestHandler() {}

RequestHandler::RequestHandler(const RequestHandler& other) { (void)other; }

RequestHandler& RequestHandler::operator=(const RequestHandler& other)
{
	(void)other;
	return (*this);
}

RequestHandler::~RequestHandler() {}
// -- OCF end --

RequestHandler::RequestHandler(Server *srv): _srv(srv) {}

// Handler for an EPOLLIN aka I/O read event.
//
// FIXME: move exception to ReqHandler class
//
// if there was a read error, or there was nothing to read, return 1 in order
// to signal deletion from epoll interest list
int RequestHandler::readRequest(int fd)
{
	std::string srv_name(_srv->getServerName());

	Logger::log_dbg2("Read request handler called!");
	if (_srv->isValidClientFd(fd) == false)
		throw(Server::ServerException("fd in handleEvent() not found"));

	Logger::log_srv(srv_name, "reading from socket" + int2str(fd));

	char		buffer[READ_BUFSIZE] = {0};
	ssize_t bytes_read					 = read(fd, buffer, READ_BUFSIZE - 1);

	if (bytes_read <= 0) {
		if (bytes_read == 0) {
			Logger::log_srv(srv_name, "Client disconnected");
			Logger::log_srv(srv_name, "closing client conn on fd " + int2str(fd));
		}
		else
			Logger::log_err(
					"read failed, errno: " + int2str(errno) + " = " + strerror(errno));
		return (REQ_ERR);
	}

	// we've read less or just enough bytes...
	if (bytes_read <= READ_BUFSIZE - 1 && _isTerminatedReq(buffer, bytes_read)) {
		std::cout << "Received:\n\n" << buffer << std::endl;
	}

	if (_isTerminatedReq(buffer, bytes_read) == false) {
		Logger::log_err("Request was not correctly terminated!");
		return (REQ_NTERM);
	}

	return (REQ_READ);
}

int RequestHandler::writeResponse(int fd)
{
	Logger::log_warn("Writing our Response!");
	const char *response =
			"HTTP/1.1 200 OK\r\nContent-Length: 40\r\n\r\nHello from José's and "
			"Frido's webserv!\n";
	ssize_t bytes_sent = send(fd, response, strlen(response), 0);
	if (bytes_sent == -1) {
		Logger::log_err("couldn't send response!");
		return (REQ_READ);
	}
	return (REQ_WRITE);
}

// check if received request was erminated with '\r\n'
bool RequestHandler::_isTerminatedReq(char *buf, ssize_t bytes_read)
{
	if (buf[bytes_read - 2] == '\r' && buf[bytes_read - 1] == '\n')
		return (true);
	return (false);
}
