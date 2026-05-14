/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 19:13:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/14 08:10:59 by fmaurer          ###   ########.fr       */
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

RequestHandler::RequestHandler() {}

RequestHandler::RequestHandler(const RequestHandler& other) { (void)other; }

RequestHandler& RequestHandler::operator=(const RequestHandler& o)
{
  (void)o;
  return (*this);
}

RequestHandler::~RequestHandler() {}
// -- OCF end --

RequestHandler::RequestHandler(Client *cli): _cli(cli)
{
  if (cli->getVsrv())
    _vsrvName = cli->getVsrv()->getName();
  else
    _vsrvName = "__VIRTUAL__";
}

// Handler for an EPOLLIN aka I/O read event aka a _Request_
//
// There are 2 possible ways a Request is being terminated:
//
//  1) The Length of the body reached the `Content-Length` Header field value
//  2) The connection is closed by the client (read == 0)
//
//  NOTE: lgtm
void RequestHandler::readRequest()
{
  std::memset(_buffer, 0, READ_BUFSIZE);
  ssize_t bytesRead = read(_cli->getFd(), _buffer, READ_BUFSIZE);

  Logger::log_srv(_vsrvName,
      "read " + int2str(bytesRead) + " bytes from " + _cli->getIfaceFdStr());

  if (bytesRead <= 0) {

    // if client closes conn i will receive a EPOLLIN event with 0 bytes read.
    // at this point the request reading should have already been finished in a
    // previous read!
    if (bytesRead == 0)
      Logger::log_srv(
          _vsrvName, "Client disco -> closing client " + _cli->getIfaceFdStr());
    else
      Logger::log_err(
          "Read failed, errno: " + int2str(errno) + " = " + getErrStr());
    _cli->setState(CLI_DISCO);
    return;
  }

  Request& req = _cli->getReq();

  if (_cli->isReading())
    req.append(_buffer, bytesRead);
  else {
    Logger::log_srv(_vsrvName, "Starting new Req");
    _cli->setReq(Request(_cli, _buffer)); // this is why **need** copy ctor!
    _cli->setState(CLI_READ);
  }

  if (req.hdrTooBig()) {
    Logger::log_srv(_vsrvName, "Header too big!", WARN);
    req.setStatusCode(HTTP_400);
  }

  // to prevent being flooded by non-sense requests check if the reqline must
  // have been received already. this will be the case after sth like 8000 bytes
  // or 2 read cycles. if this yields non-sense we can directly quit.
  if (req.reqlineReceived())
    req.parseReqLine();

  // if we have got the full header - terminated by 2x CRLF - we will
  // immediately parse that in order to get
  //
  //  a) a possible Content-Length field
  //  b) a possible Host header for Virtual Server routing
  if (!req.badRequest() && req.hdrComplete()) {
    if (req.parseReqHeaders() == HTTP_400)
      Logger::logDbg1(
          "RequestHandler::readRequest", "400 from Req::parseHeaders");
    else
      _setVirtualServerFromHeader();
  }

  if (req.reqComplete() || req.badRequest()) {
    Logger::log_reqres(_vsrvName, "Processing Request...", req.getReqstr());
    _cli->setState(CLI_SEND);
    req.processReq();
  }
}

// using the `response.data()` here to work in binary mode. this, very much like
// `std::string::c_str()` returns a `const char` pointer to the data stored in
// the string. Also using `std::string::size()` which returns the size in raw
// bytes
//
// When this function is called there are 2 possibilities:
//
//    1) The Request was completed normally. Then also Request::processReq was
//       called and we will have a generated Response stored in Request::_respo
//       which we can send.
//
//    2) The Request was not processed normally. There are several reasons for
//       this:
//
//      a) Bad req -> there were hdr errors. So no route is matched and only if
//      the client is not virtual we might look up the
//
void RequestHandler::writeResponse()
{
  str          response;
  e_HTTPStatus statusCode;

  // QUESTION what happens here if a client connected but never sent anything?
  // ANSWER timeout. the clients req object will have been default contructed.
  // that is reqComplete == false.
  Request& req = _cli->getReq();

  if (_cli->isTimeout()) {

    Logger::log_dbg1("RequestHandler: 408 due to timeout");
    statusCode = HTTP_408;

    if (_cli->isVirtual())
      response = Response::genDefaultErrResponse(statusCode);
    else
      // TODO add proper errorPage from server scope getting here
      response = Response::genDefaultErrResponse(statusCode);
  }
  else {

    statusCode = req.getStatusCode();
    response   = req.getResponseStr();
  }

  Logger::log_srv(_cli->getVsrv()->getName(),
      "Sending Response (" + int2str(statusCode) + ") to " +
          _cli->getIfaceFdStr());
  Logger::log_reqres(_cli->getVsrv()->getName(), "Response", response);

  if (response.empty())
    throw ReqHandlerException("Cannot write response! Nothing to write!");

  ssize_t bytes_sent = send(_cli->getFd(), response.data(), response.size(), 0);

  // QUESTION: can we somehow provoke this for testing?
  if (bytes_sent == -1) {
    Logger::log_err("Couldn't send response!");
    return;
  }

  // if some error occurred or Connection: close -> disco
  if ((statusCode >= HTTP_400 && statusCode != HTTP_404) || req.closeConn())
    _cli->setState(CLI_DISCO);
  else
    _cli->setState(CLI_IDLE);

  Logger::log_srv(_cli->getVsrv()->getName(), "Response successfully sent!");

  _cli->resetReq();
}

RequestHandler::ReqHandlerException::ReqHandlerException(
    const std::string& msg): std::runtime_error("ReqHandlerException: " + msg)
{}

void RequestHandler::setVsrvName(const str& n) { _vsrvName = n; }

// FIXME: maybe change logging to dbg1 for prod
// TODO: split away a possible 'https://' or port part, aka: isolate the fqdn
void RequestHandler::_setVirtualServerFromHeader()
{
  if (_vsrvName == "__VIRTUAL__") {

    std::map<str, str>& hdrs = _cli->getReq().getHeaders();

    // FIXME is lowercasing alright here?
    if (hdrs.find("host") != hdrs.end()) {
      str host = hdrs["host"];

      for (std::vector<VServer *>::iterator it =
               _cli->getPotentialVsrvs().begin();
          it != _cli->getPotentialVsrvs().end();
          it++)
      {
        Logger::log_dbg0("checking this potential server: " + (*it)->getName());
        if ((*it)->getName() == host) {
          Logger::log_dbg0("Found matching VServer for host = " + host);
          _cli->setVsrv(*it);
          (*it)->addClient(_cli);
          _vsrvName = host;
          return;
        }
      }
    }

    Logger::log_warn("RequestHandler::_setVirtualServerFromHeader",
        "No matching vSrv found. Moving Req to default vSrv.");

    VServer *srv = _cli->getPotentialVsrvs()[0];
    _cli->setVsrv(srv);
    srv->addClient(_cli);
    _vsrvName = srv->getName();

    _cli->getReq().evaluateTarget();
  }
}

str RequestHandler::_getErrPage(e_HTTPStatus c)
{
  (void)c;
  return "";
}
