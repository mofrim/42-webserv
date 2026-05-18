/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/18 19:13:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/18 21:31:48 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Request.hpp"
#include "RequestHandler.hpp"
#include "VServer.hpp"
#include "utils.hpp"

#include <cstring>
#include <unistd.h>

// --------------------------------=[ OCF ]=-------------------------------- //

RequestHandler::RequestHandler() {}

RequestHandler::RequestHandler(const RequestHandler& o) { (void)o; }

RequestHandler& RequestHandler::operator=(const RequestHandler& o)
{
  (void)o;
  return (*this);
}

RequestHandler::~RequestHandler() {}

// ------------------------------=[ the rest ]=------------------------------ //

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
void RequestHandler::readRequest()
{
  std::memset(_buffer, 0, READ_BUFSIZE);

  ssize_t bytesRead = read(_cli->getFd(), _buffer, READ_BUFSIZE);

  Logger::logSrv(_vsrvName,
      "read " + int2str(bytesRead) + " bytes from " + _cli->getIfaceFdStr());

  if (bytesRead <= 0) {

    // if client closes conn i will receive a EPOLLIN event with 0 bytes read.
    // at this point the request reading should have already been finished in a
    // previous read!
    if (bytesRead == 0)
      Logger::logSrv(
          _vsrvName, "Client disco -> closing client " + _cli->getIfaceFdStr());
    else
      // Dear evalutor. I _AM_ checking errno here, but only for _LOGGING_ not
      // for "adjusting server behavior".
      Logger::logErr("Read failed with: " + getErrnoStr() + " -> disco!");
    _cli->setState(CLI_DISCO);
    return;
  }

  Request& req = _cli->getReq();

  // first time we come here cli will be in CLI_IDLE state, so a new Req is
  // started
  if (_cli->isReading() || _cli->isDraining())
    req.append(_buffer, bytesRead);
  else {
    Logger::logSrv(_vsrvName, "Starting new Req");
    _cli->setReq(Request(_cli, _buffer, bytesRead));
    _cli->setState(CLI_READ);
  }

  if (req.hdrTooBig()) {
    Logger::logSrv(_vsrvName, "Header too big!", WARN);
    req.setStatusCode(HTTP_400);
  }

  // to prevent being flooded by non-sense requests check if the reqline must
  // have been received already. this will be the case after sth like 8000 bytes
  // or 2 read cycles. if this yields non-sense we can directly quit with 400
  if (req.reqlineReceived() && !req.reqlineParsed()) {
    Logger::logDbg1("RequestHandler::readRequest", "Parsing reqline...");
    req.parseReqLine();
  }

  // if we have got the full header - terminated by 2x CRLF - we will
  // immediately parse that in order to get
  //
  //  a) a possible Content-Length field
  //  b) a possible Host header for Virtual Server routing
  //
  //  if the hdrs were already parsed, i.e. we are reading a body to some POST
  //  req, we don't do anyhing here.
  if (!req.badRequest() && !req.hdrsParsed() && req.hdrComplete()) {
    if (req.parseReqHeaders() >= HTTP_400)
      Logger::logSrv(_vsrvName,
          "Req::parseHeaders returned " + int2str(req.getStatus()),
          WARN);
    else
      _setVirtualServerFromHeader();
  }

  if (req.reqComplete() || req.badRequest()) {
    Logger::logReqRes(_vsrvName, "Processing Request", req.getReqstr());
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

    Logger::logDbg1("RequestHandler: 408 due to timeout");
    statusCode = HTTP_408;

    if (_cli->isCGIing())
      req.getRespo().cgiCleanupFds();

    if (_cli->isVirtual() && _cli->getVsrv() == NULL)
      response = Response::genDefaultErrResponse(statusCode);
    else
      // TODO add proper errorPage from server scope getting here
      response = Response::genDefaultErrResponse(statusCode);
  }
  else {

    statusCode = req.getStatus();
    response   = req.getResponseStr();
  }

  Logger::logSrv(_cli->getVsrv()->getName(),
      "Sending Response (" + int2str(statusCode) + ") to " +
          _cli->getIfaceFdStr());
  Logger::logReqRes(_cli->getVsrv()->getName(), "Response", response);

  if (response.empty())
    throw ReqHandlerException("Cannot write response! Nothing to write!");

  ssize_t bytes_sent = send(_cli->getFd(), response.data(), response.size(), 0);

  // keep on sending if non-blocking send did fail.
  // QUESTION: can we somehow provoke this for testing?
  if (bytes_sent < 0 || static_cast<size_t>(bytes_sent) != response.size()) {
    Logger::logErr("Couldn't send (complete) response!");
    return;
  }

  // if some error occurred or Connection: close -> disco
  if ((statusCode >= HTTP_400 && statusCode != HTTP_404) || req.closeConn())
    _cli->setState(CLI_DISCO);
  else
    _cli->setState(CLI_IDLE);

  Logger::logSrv(_cli->getVsrv()->getName(), "Response successfully sent!");

  _cli->resetReq();
}

RequestHandler::ReqHandlerException::ReqHandlerException(
    const std::string& msg): std::runtime_error("ReqHandlerException: " + msg)
{}

void RequestHandler::setVsrvName(const str& n) { _vsrvName = n; }

void RequestHandler::_setVirtualServerFromHeader()
{
  if (_cli->isVirtual()) {

    std::map<str, str>& hdrs = _cli->getReq().getHeaders();

    if (hdrs.find("host") != hdrs.end()) {
      u16              port = 0;
      str              host;
      std::vector<str> hsplit = splitString(hdrs["host"], ":");
      if (hsplit.size() == 2)
        port = str2u16(hsplit[1]);
      if (hsplit.size() >= 1)
        host = hsplit[0];
      else
        host = hdrs["host"];

      // client requesting to the same virtual server as before?
      if (_cli->getVsrv() != NULL && host == _cli->getVsrv()->getName()) {
        _cli->getReq().evaluateTarget();
        return;
      }

      // client requesting to a different vsrv
      else if (_cli->getVsrv() != NULL) {
        if (_cli->getVsrv()->removeVirtualClient(_cli) == 0)
          throw std::runtime_error(
              "(RequestHandler::_setVirtualServerFromHeader) failed to remove "
              "cli from prev vsrv");
      }

      for (std::vector<VServer *>::iterator it =
               _cli->getPotentialVsrvs().begin();
          it != _cli->getPotentialVsrvs().end();
          it++)
      {
        if ((*it)->getName() == host) {
          Logger::logSrv(
              "__VIRTUAL__", "Found matching VServer for host = " + host);
          _cli->setVsrv(*it);
          _cli->setVsrvPort(port);
          (*it)->addVirtualClient(_cli);
          _vsrvName = host;
          return;
        }
      }
    }

    Logger::logWarn("RequestHandler::_setVirtualServerFromHeader",
        "No matching vSrv found. Moving Req to default vSrv.");

    VServer *srv = _cli->getPotentialVsrvs()[0];
    _cli->setVsrv(srv);
    srv->addVirtualClient(_cli);
    _vsrvName = srv->getName();

    _cli->getReq().evaluateTarget();
  }
}
