/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 23:39:57 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/20 18:20:46 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ErrPages.hpp"
#include "Logger.hpp"
#include "Request.hpp"

#include <iostream>

// -- OCF --

Request::Request(): _srvcfg(NULL), _reqstr("")
{}

Request::Request(const Request& other)
{
  if (this != &other) {
    _srvcfg   = other._srvcfg;
    _reqstr   = other._reqstr;
    _response = other._response;
  }
}

Request& Request::operator=(const Request& other)
{
  if (this != &other) {
    _srvcfg   = other._srvcfg;
    _reqstr   = other._reqstr;
    _response = other._response;
  }
  return (*this);
}

Request::~Request()
{}
// -- OCF end --

// the standard ctor we use for initializing a request _and_ parse the request
// at the same time.
Request::Request(const VServerCfg *scfg, const std::string& reqstr)
{
  _srvcfg = scfg;
  _reqstr = reqstr;
  Logger::log_reqres("Request", _reqstr);
  _parseRequest();
}

// TODO: a. lot. of. work. to. be. done. in. here!
// TODO: in Repsonse class: construct Response string by appending "\r\n" aka
// CRLF to each line of the response string.
// TODO: automate response body length calculation
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
        "Server: m0fr1m-webserv\r\n"
        "Date: Mon, 05 Jan 2026 07:08:57 GMT\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 157\r\n"
        "Connection: close\r\n"
        "\r\n" +
        ErrPages::getDefaultErrPage(400);
  }
  else
    _response =
        "HTTP/1.1 200 OK\r\nContent-Length: 29\r\nContent-Type: text/plain; "
        "charset=utf-8\r\n\r\nHello from m0fr1m's webserv!\n";
}

std::string Request::getResponse() const
{
  return (_response);
}

// check if received request was erminated with '\r\n'
bool Request::_isTerminatedReq()
{
  if (_reqstr.size() < 2)
    return (false);
  if (_reqstr.compare(_reqstr.size() - 2, 2, "\r\n") == 0)
    return (true);
  return (false);
}
