/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 19:13:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/01/05 08:28:06 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Request.hpp"
#include "RequestHandler.hpp"
#include "Server.hpp"
#include "Webserv.hpp"
#include "utils.hpp"

#include <errno.h>
#include <string.h>
#include <unistd.h>

// -- OCF --

RequestHandler::RequestHandler() {}

RequestHandler::RequestHandler(const RequestHandler& other) { (void)other; }

RequestHandler& RequestHandler::operator=(const RequestHandler& other)
{
	if (this != &other) {
		_srv			= other._srv;
		_requests = other._requests;
	}
	return (*this);
}

RequestHandler::~RequestHandler() {}
// -- OCF end --

RequestHandler::RequestHandler(Server *srv): _srv(srv) {}

// Handler for an EPOLLIN aka I/O read event aka a _Request_
//
// FIXME: move exception to ReqHandler class
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
			Logger::log_srv(srv_name, "-> closing client conn on fd " + int2str(fd));
		}
		else
			Logger::log_err(
					"read failed, errno: " + int2str(errno) + " = " + strerror(errno));
		return (REQ_ERR);
	}

	// NEXT: José this is where you join in!
	//
	// we've read less or just enough bytes... let's process the Request!!!
	// add the new request _to the front_ of _requests vector bc we will send
	// response by FIFO principle using pop_back()
	//
	// FIXME: actually, we only have to save the response in the vector here. or
	// do we need a vector here at all?!
	Request newReq(_srv->getCfg(), buffer);
	_requests.insert(_requests.begin(), newReq);

	return (REQ_READ);
}

// the main routine responsible for sending the response off to the cient!
// taking the response from the back of the _requests vector as new requests
// will be pushed to the front (see above)
int RequestHandler::writeResponse(int fd)
{
	if (_requests.empty())
		throw(ReqHandlerException("Cannot write response! Ain't got no requests!"));
	Logger::log_msg("Writing our Response!");
	std::string response = _requests.back().getResponse();
	Logger::log_reqres("Response", response);
	ssize_t bytes_sent = send(fd, response.c_str(), strlen(response.c_str()), 0);
	if (bytes_sent == -1) {
		Logger::log_err("couldn't send response!");
		return (REQ_READ);
	}
	_requests.pop_back();
	return (REQ_WRITE);
}

RequestHandler::ReqHandlerException::ReqHandlerException(
		const std::string& msg): std::runtime_error("ReqHandlerException: " + msg)
{}
