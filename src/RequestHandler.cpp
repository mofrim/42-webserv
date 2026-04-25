/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 19:13:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/25 11:38:40 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Request.hpp"
#include "RequestHandler.hpp"
#include "VServer.hpp"
#include "utils.hpp"

#include <errno.h>
#include <unistd.h>

// -- OCF --

RequestHandler::RequestHandler()
{}

RequestHandler::RequestHandler(const RequestHandler& other)
{
  (void)other;
}

RequestHandler& RequestHandler::operator=(const RequestHandler& other)
{
  if (this != &other) {
    _srv      = other._srv;
    _requests = other._requests;
  }
  return (*this);
}

RequestHandler::~RequestHandler()
{}
// -- OCF end --

RequestHandler::RequestHandler(VServer *srv): _srv(srv)
{}

// Handler for an EPOLLIN aka I/O read event aka a _Request_
//
// There are 2 possible ways a Request is being terminated:
//
//  1) The Length of the body reached the `Content-Length` Header field value
//  2) The connection is closed by the client (read == 0)
//
// FIXME: move exception to ReqHandler class
//
int RequestHandler::readRequest(Client *cli)
{
  str srv_name(_srv->getServerName());
  int fd = cli->getFd();

  Logger::log_dbg2("Read request handler called!");
  if (_srv->isValidClientFd(fd) == false)
    throw(VServer::ServerException("fd in handleEvent() not found"));

  Logger::log_srv(srv_name, "reading from socket" + int2str(fd));

  char    buffer[READ_BUFSIZE + 1] = {0};
  ssize_t bytes_read               = read(fd, buffer, READ_BUFSIZE);

  Logger::log_srv(srv_name, "read " + int2str(bytes_read) + " bytes");

  if (bytes_read <= 0) {

    // if client closes conn i will receive a EPOLLIN event with 0 bytes read.
    // at this point the request reading should have already been finished in a
    // previous read!
    if (bytes_read == 0) {
      Logger::log_srv(
          srv_name, "Client disco -> closing client on fd " + int2str(fd));
      return (REQ_DISCO);
    }
    else
      Logger::log_err(
          "read failed, errno: " + int2str(errno) + " = " + getErrStr());

    return (REQ_ERR);
  }

  // FIXME:
  // QUESTION: this is certainly not correctly organized here. how many
  // simultaneous reqs from one client can we have? in theory we can have
  // mutiple ongoing CGI reqs. but only one simple req at a time. i guess
  // TODO: clarify!
  //
  // append the read to the first unfinished Req we find. If there is no
  // unfinished Req add a new one to the queue.
  if (_cliHasUnfinishedRequest(cli))
    _getUnfinishedReq(cli).append(buffer);
  else {
    Logger::log_srv(srv_name, "Adding new Req to queue");
    Request newReq(_srv, cli, buffer);
    _reqQueue[cli].push_front(newReq);
  }

  if (_reqQueue[cli].front().reqComplete()) {
    Logger::log_reqres("Request", _reqQueue[cli].front().getReqstr());
    _reqQueue[cli].front().setFinished();
    return REQ_WRITE;
  }
  return REQ_INC;
}

// the main routine responsible for sending the response off to the cient!
// taking the response from the back of the _requests vector as new requests
// will be pushed to the front (see above)
//
// using the `response.data()` here to work in binary mode. this, very much like
// `std::string::c_str()` returns a `const char` pointer to the data stored in
// the string. Also using `std::string::size()` which returns the size in raw
// bytes
int RequestHandler::writeResponse(Client *cli)
{
  int ret = REQ_READ;

  if (_reqQueue.empty())
    throw(ReqHandlerException("Cannot write response! Ain't got no requests!"));

  Request *req = &_reqQueue[cli].back();

  // if some error occurred -> disco.
  if (req->getStatusCode() >= HTTP_400)
    ret = REQ_DISCO;

  std::string response = _reqQueue[cli].back().getResponseStr();

  // Logger::log_reqres("webserv's response", response);

  ssize_t bytes_sent = send(cli->getFd(), response.data(), response.size(), 0);

  if (bytes_sent == -1) {
    Logger::log_err("couldn't send response!");
    return REQ_WRITE;
  }
  _reqQueue[cli].pop_back();
  if (_reqQueue[cli].empty())
    _reqQueue.erase(cli);

  return ret;
}

RequestHandler::ReqHandlerException::ReqHandlerException(
    const std::string& msg): std::runtime_error("ReqHandlerException: " + msg)
{}

bool RequestHandler::_cliHasUnfinishedRequest(Client *cli)
{
  if (_reqQueue.find(cli) != _reqQueue.end()) {
    Logger::log_msg("RequestHandler: found " + int2str(_reqQueue[cli].size()) +
        " pending requests for cli");
    for (std::deque<Request>::iterator it = _reqQueue[cli].begin();
        it != _reqQueue[cli].end();
        ++it)
      if (!it->isFinished())
        return true;
  }
  return false;
}

// FIXME: maybe i will only need this function and then check for
// _reqQueue.end()
Request& RequestHandler::_getUnfinishedReq(Client *cli)
{
  std::deque<Request>::iterator it = _reqQueue[cli].begin();
  for (; it != _reqQueue[cli].end(); ++it)
    if (!it->isFinished())
      break;
  return *it;
}
