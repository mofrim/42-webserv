/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:36:43 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/16 22:29:21 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include "Logger.hpp"
#include "VServer.hpp"
#include "Webserv.hpp"

// FIXME: remove cause sleep is not an allowed function!
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <utils.hpp>

Webserv::Webserv(): _defaultCfg(true), _shutdown_server(false), _numOfClients(0)
{
  _Webserv.setServerName("Webserv");
}

Webserv::Webserv(const Webserv& other) { (void)other; }

Webserv& Webserv::operator=(const Webserv& other)
{
  (void)other;
  return (*this);
}

// nothing to be done here, so far..
Webserv::~Webserv() {}

// TODO: there will be much more to do in here. What?
//
// - close sockets
// - disconnect clients
// - etc....
void Webserv::shutdownWebserv()
{
  Logger::logWarn("shutting down webserv...");
  _shutdown_server = true;
}

// the Idea is: keep the existence of the Config object limited to this method.
// After this method there should be only the vector<VServer> which then holds
// all the data.
void Webserv::readConfig(const str& cfgFilename)
{
  _defaultCfg = false;

  ConfigParser parsy(cfgFilename);

  if (parsy.bad())
    throw WebservInitException("Error reading cfgfile!");

  try {
    parsy.parse();
  } catch (const std::exception& e) {
    throw;
  }
  const std::vector<VServerCfg>& cfgs = parsy.getCfgs();

  if (cfgs.empty()) {
    Logger::logWarn(
        "Webserv::readConfig", "Could not read any vsrvs from cfg!");
    return;
  }

  for (std::vector<VServerCfg>::const_iterator it = cfgs.begin();
      it != cfgs.end();
      ++it)
    _vservers.push_back(VServer(*it));
}

// The main routine for setting up the servers listed in the Config.
void Webserv::_setupServers()
{
  if (_defaultCfg)
    _initDefaultCfg();

  std::vector<VServer>::iterator it = _vservers.begin();
  while (it != _vservers.end()) {
    _setupSingleServer(it);
    if (it->isInitFailed())
      it = _vservers.erase(it);
    else
      ++it;
  }
  Logger::logDbg2("Number of not-failed Servers left after cleanup: " +
      int2str(_vservers.size()));
  if (_vservers.size() == 0) {
    Logger::logErr("Could not setup any Server!");
    shutdownWebserv();
  }
}

void Webserv::_setupSingleServer(std::vector<VServer>::iterator srvIt)
{
  Logger::logMsg("Trying to setup server '" + srvIt->getName() + "':");
  try {
    // init() needs the _vservers.begin and srvIt
    srvIt->init(_vservers.begin(), srvIt);
    for (std::set<int>::const_iterator it = srvIt->getListenFds().begin();
        it != srvIt->getListenFds().end();
        it++)
      _vserverFdMap[*it].push_back(&*srvIt);
  } catch (const VServer::ServerInitException& e) {
    Logger::logWarn(
        "Setting up server " + srvIt->getName() + " failed: " + e.what());
    srvIt->setSetupFailed();
  }
  if (!srvIt->isInitFailed())
    srvIt->printCfg();
}

// nothing to do here so far...
void Webserv::_shutdownAllServers()
{
  Logger::logMsg("Bye-bye from m0fr1m's webserv!");
}

// TODO: figure out the best timeout for epoll_wait. For now -1 is okay. but
// even a small timeout like 10ms might be okay. what are the benefits here?
//
// NOTE: i think checking errno here is okay as the subject only demands not
// checking it after read write.
//
// the logic here is simple: if we are getting an I/O event on one of our
// servers listening sockets (aka ports) this is a connection request ->
// add a new client!
//
// EINTR == epoll_wait was interrupted by signal_handler being calles (expl:
// epoll_wait is a syscall and as such atomic. so if any interupt is being
// called the syscall exits)
void Webserv::run()
{
  _setupServers();
  _epoll.setup(_vservers);

  // the main loop
  while (_shutdown_server == false) {
    int nfds = _epoll.wait();
    if (nfds == -1 && errno != EINTR)
      throw(WebservRunException("epoll_wait failed"));

    // DEBUG
    // _epoll.printReadylist();

    for (int eventIdx = 0; eventIdx < nfds; ++eventIdx) {
      int currentFd = _epoll.getEventFd(eventIdx);

      // 1) new connection
      if (_isServerFd(currentFd) && _numOfClients < MAX_CLIENTS) {
        std::vector<VServer *> vsrvs = _getServersByFd(currentFd);

        if (vsrvs.empty()) {
          Logger::logWarn("Webserv::run: _getServerByFd returned no servers");
          continue;
        }

        Client *cli;
        if (vsrvs.size() == 1)
          cli = vsrvs[0]->addClient(currentFd);
        else {
          Logger::logErr("MULTISERVER BRANCH CALLED!!!");
          if ((cli = Client::newVirtualCli(currentFd)) == NULL) {
            Logger::logWarn(
                "Webserv::run: Client::newCliServerless returned NULL");
            continue;
          }
          cli->setPotentialVsrvs(vsrvs);
        }
        _fdClientMap[cli->getFd()] = cli;
        _epoll.addClient(cli->getFd());
        _numOfClients++;
        Logger::drawCycleSep();
      }

      // 2) existing connection
      //
      // QUESTION: what if _numOfClients >= MAX_CLIENTS ?!?! we need to handle
      // this!!! Also:
      // FIXME: clarify how we handle connection keepalive with clients
      else {
        Client  *cli  = _fdClientMap[currentFd];
        VServer *vsrv = cli->getVsrv();
        u32      ev   = _epoll.getEvent(eventIdx);

        // skip cli if event is EPOLLOUT but the client has got nothing to send
        // or is still reading a request
        if ((cli->isIdling() || cli->isReading()) && (ev & EPOLLOUT))
          continue;

        cli->handleEvent(ev);

        if (cli->isDisco()) {
          _epoll.removeClient(currentFd);
          if (vsrv)
            vsrv->deleteClient(currentFd);
          else
            delete cli;
          _numOfClients--;
          _fdClientMap.erase(currentFd);
        }
        else if (cli->isSending())
          _epoll.modifyClient(currentFd, EPOLLOUT);
        else if (cli->isReading() || cli->isIdling() || cli->isDraining())
          _epoll.modifyClient(currentFd, EPOLLIN);
      }
      _timeoutClients();
      Logger::drawCycleSep();
    }
  }
  _epoll.closeEpollFd();
}

// For serverless clients there can only be a EPOLLIN event bc it is the first
// real request that is processed
// TODO implement this
//
// FIXME DEAD CODE ?!?!?!
// FIXME REMOVE ?!?!?!?
void Webserv::_handleEventServerless(u32 ev, Client *cli)
{
  if (ev & EPOLLIN) {
    return cli->handleEvent(ev);
  }
  Logger::logErr("Webserv::handleEventServerless: srvless event not EPOLLIN!");
  cli->setState(CLI_DISCO);
}

// TODO: add special treatment for CGI clients here
void Webserv::_timeoutClients()
{
  time_t now = time(NULL);

  std::map< int, Client * >::iterator it = _fdClientMap.begin();
  while (it != _fdClientMap.end()) {
    Client *cli = it->second;
    if (difftime(now, cli->getLastActive()) > WsrvLib::WsrvSettings.reqTimeout)
    {
      cli->timeout();
      _epoll.modifyClient(cli->getFd(), EPOLLOUT);
      cli->setState(CLI_SEND);
    }
    it++;
  }
}

// -----------------------------=[ Exceptions ]=----------------------------- //

Webserv::WebservInitException::WebservInitException(const std::string& msg):
  std::runtime_error("WebservInitException: " + msg)
{}

Webserv::WebservRunException::WebservRunException(const std::string& msg):
  std::runtime_error("WebservRunException: " + msg)
{}
