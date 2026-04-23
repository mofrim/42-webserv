/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 23:39:57 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/23 13:32:44 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ErrPages.hpp"
#include "Logger.hpp"
#include "Request.hpp"

// --------------------------------=[ OCF ]=-------------------------------- //

Request::Request(): _vsrv(NULL), _reqstr(""), _reqFinished(false)
{}

Request::Request(const Request& o)
{
  if (this != &o) {
    _vsrv        = o._vsrv;
    _reqstr      = o._reqstr;
    _response    = o._response;
    _reqFinished = o._reqFinished;
  }
}

Request& Request::operator=(const Request& o)
{
  if (this != &o) {
    _vsrv        = o._vsrv;
    _reqstr      = o._reqstr;
    _response    = o._response;
    _reqFinished = o._reqFinished;
  }
  return (*this);
}

Request::~Request()
{}

// ------------------------------=[ END OCF ]=------------------------------ //

// the standard ctor we use for initializing a request _and_ parse the request
// at the same time.
Request::Request(VServer *vsrv, Client *cli, const std::string& reqstr)
{
  _vsrv        = vsrv;
  _cli         = cli;
  _reqstr      = reqstr;
  _reqFinished = false;
  // FIXME: you don't belong here!
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

// check if received request was terminated with '\r\n'
bool Request::_isTerminatedReq()
{
  if (_reqstr.size() < 4)
    return (false);
  if (_reqstr.compare(_reqstr.size() - 4, 4, "\r\n\r\n") == 0)
    return (true);
  return (false);
}

bool Request::isFinished() const
{
  return _reqFinished;
}

void Request::setFinished()
{
  _reqFinished = true;
}

void Request::append(const str& s)
{
  _reqstr += s;
  Logger::log_reqres("Appending to req:", s);
}

bool Request::hdrComplete() const
{
  if (_reqstr.rfind("\r\n\r\n") != str::npos)
    return true;
  return false;
}

// for now we only look at the hdr
bool Request::reqComplete()
{
  if (hdrComplete()) {
    Logger::log_msg("Request::reqComplete: Request complete!");
    return true;
  }
  return false;
}

e_Method Request::getMethod() const
{
  if (hdrComplete())
    return _reqline.method;
  return M_UNKNOWN;
}

const str& Request::getReqstr() const
{
  return _reqstr;
}
