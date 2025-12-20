/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 23:39:57 by fmaurer           #+#    #+#             */
/*   Updated: 2025/12/20 16:55:23 by fmaurer          ###   ########.fr       */
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
void Request::_parseRequest()
{
	_response =
			"HTTP/1.1 200 OK\r\nContent-Length: 40\r\n\r\nHello from José's and "
			"Frido's webserv!\n";
}

std::string Request::getResponse() const { return (_response); }
