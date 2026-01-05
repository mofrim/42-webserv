/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 23:39:57 by fmaurer           #+#    #+#             */
/*   Updated: 2026/01/05 08:50:35 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Request.hpp"

#include <iostream>

// -- OCF --

Request::Request(): _srvcfg(NULL), _reqstr("") {}

Request::Request(const Request& other)
{
	if (this != &other) {
		_srvcfg		= other._srvcfg;
		_reqstr		= other._reqstr;
		_response = other._response;
	}
}

Request& Request::operator=(const Request& other)
{
	if (this != &other) {
		_srvcfg		= other._srvcfg;
		_reqstr		= other._reqstr;
		_response = other._response;
	}
	return (*this);
}

Request::~Request() {}
// -- OCF end --

// the standard ctor we use for initializing a request _and_ parse the request
// at the same time.
Request::Request(const ServerCfg *scfg, const std::string& reqstr)
{
	_srvcfg = scfg;
	_reqstr = reqstr;
	Logger::log_reqres("Request", _reqstr);
	_parseRequest();
}

// TODO: a. lot. of. work. to. be. done. in. here!
// TODO: in Repsonse class: construct Response string by appending "\r\n" aka
// CRLF to each line of the response string.
//
// Important resource:
//
// 	https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Status
void Request::_parseRequest()
{

	if (!_isTerminatedReq()) {
		_statusCode = 400;
		_response =
				"HTTP/1.1 400 Bad Request\r\n"
				"Server: nginx/1.28.0\r\n"
				"Date: Mon, 05 Jan 2026 07:08:57 GMT\r\n"
				"Content-Type: text/html\r\n"
				"Content-Length: 157\r\n"
				"Connection: close\r\n"
				"\r\n"
				"<html>\r\n"
				"<head><title>400 Bad Request</title></head>\r\n"
				"<body>\r\n"
				"<center><h1>400 Bad Request</h1></center>\r\n"
				"<hr><center>nginx/1.28.0</center>\r\n"
				"</body>\r\n"
				"</html>\r\n";
	}
	else
		_response =
				"HTTP/1.1 200 OK\r\nContent-Length: 40\r\n\r\nHello from José's and "
				"Frido's webserv!\n";
}

std::string Request::getResponse() const { return (_response); }

// check if received request was erminated with '\r\n'
bool Request::_isTerminatedReq()
{
	if (_reqstr.size() < 2)
		return (false);
	if (_reqstr.compare(_reqstr.size() - 2, 2, "\r\n") == 0)
		return (true);
	return (false);
}
