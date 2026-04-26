/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 19:13:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/26 20:24:59 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Request.hpp"
#include "RequestHandler.hpp"
#include "VServer.hpp"
#include "utils.hpp"

#include <cstring>
#include <errno.h>
#include <unistd.h>

// -- OCF --

RequestHandler::RequestHandler()
{}

RequestHandler::RequestHandler(const RequestHandler& other)
{
  (void)other;
}

RequestHandler& RequestHandler::operator=(const RequestHandler& o)
{
  (void)o;
  return (*this);
}

RequestHandler::~RequestHandler()
{}
// -- OCF end --

RequestHandler::RequestHandler(Client *cli):
  _cli(cli), _vsrvName(cli->getVsrv()->getName())
{}

// Handler for an EPOLLIN aka I/O read event aka a _Request_
//
// There are 2 possible ways a Request is being terminated:
//
//  1) The Length of the body reached the `Content-Length` Header field value
//  2) The connection is closed by the client (read == 0)
void RequestHandler::readRequest()
{
  int fd = _cli->getFd();

  Logger::log_srv(_vsrvName, "reading from " + _cli->getIfaceFdStr());

  memset(_buffer, 0, READ_BUFSIZE);
  ssize_t bytes_read = read(fd, _buffer, READ_BUFSIZE);

  Logger::log_srv(_vsrvName, "read " + int2str(bytes_read) + " bytes");

  if (bytes_read <= 0) {

    // if client closes conn i will receive a EPOLLIN event with 0 bytes read.
    // at this point the request reading should have already been finished in a
    // previous read!
    if (bytes_read == 0)
      Logger::log_srv(
          _vsrvName, "Client disco -> closing client " + _cli->getIfaceFdStr());
    else
      Logger::log_err(
          "Read failed, errno: " + int2str(errno) + " = " + getErrStr());
    _cli->setState(CLI_DISCO);
    return;
  }

  if (_cli->isReading())
    _cli->getReq().append(_buffer);
  else {
    Logger::log_srv(_vsrvName, "Starting new Req");
    _cli->setReq(Request(_cli, _buffer));
    _cli->setState(CLI_READ);
  }

  if (_cli->isReqComplete()) {
    Logger::log_reqres(
        _vsrvName, "Request complete", _cli->getReq().getReqstr());
    _cli->setReqFinished();
    _cli->setState(CLI_SEND);
  }
}

// the main routine responsible for sending the response off to the cient!
// taking the response from the back of the _requests vector as new requests
// will be pushed to the front (see above)
//
// using the `response.data()` here to work in binary mode. this, very much like
// `std::string::c_str()` returns a `const char` pointer to the data stored in
// the string. Also using `std::string::size()` which returns the size in raw
// bytes
void RequestHandler::writeResponse()
{
  str response;
  u16 statusCode;

  if (_cli->isTimeout()) {
    statusCode = HTTP_408;
    Response r;
    r.genErrResponse(HTTP_408);
    response = r.getRespoStr();
  }
  else {
    response   = _cli->getReq().getResponseStr();
    statusCode = _cli->getReq().getStatusCode();
  }

  Logger::log_srv(_cli->getVsrv()->getName(),
      "Sending our Response to " + _cli->getIfaceFdStr());

  if (response.empty())
    throw(ReqHandlerException("Cannot write response! Nothing to write!"));

  ssize_t bytes_sent = send(_cli->getFd(), response.data(), response.size(), 0);

  if (bytes_sent == -1) {
    Logger::log_err("Couldn't send response!");
    return;
  }

  // if some error occurred -> disco.
  if (statusCode >= HTTP_400 && statusCode != HTTP_404)
    _cli->setState(CLI_DISCO);
  else
    _cli->setState(CLI_IDLE);

  Logger::log_srv(_cli->getVsrv()->getName(), "Response successfully sent!");
  _cli->resetReq();
}

RequestHandler::ReqHandlerException::ReqHandlerException(
    const std::string& msg): std::runtime_error("ReqHandlerException: " + msg)
{}

void RequestHandler::setVsrvname(const str& n)
{
  _vsrvName = n;
}
