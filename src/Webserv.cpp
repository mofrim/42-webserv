/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:36:43 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/25 12:24:24 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include "Logger.hpp"
#include "VServer.hpp"
#include "Webserv.hpp"

// FIXME: remove cause sleep is not an allowed function!
#include <errno.h>
#include <unistd.h>
#include <utils.hpp>

Webserv::Webserv():
  _defaultCfg(true), _shutdown_server(false), _numOfServers(0), _numOfClients(0)
{
  _Webserv.setServerName("Webserv");
}

Webserv::Webserv(const Webserv& other)
{
  (void)other;
}

Webserv& Webserv::operator=(const Webserv& other)
{
  (void)other;
  return (*this);
}

// nothing to be done here, so far..
Webserv::~Webserv()
{}

// TODO: there will be much more to do in here. What?
//
// - close sockets
// - disconnect clients
// - etc....
void Webserv::shutdownWebserv()
{
  Logger::log_warn("shutting down webserv...");
  _shutdown_server = true;
}

// the Idea is: keep the existence of the Config object limited to this method.
// After this method there should be only the vector<Server> which then holds
// all the data. Additionally there can be some global config options which can
// be saved in Webserv class or also in Server class.
void Webserv::getServersFromCfg(const std::string& cfgFilename)
{
  (void)cfgFilename;
  Config cfg;

  try {
    cfg.parseCfgFile(cfgFilename);
  } catch (const std::exception& e) {
    throw(e);
  }

  _numOfServers = cfg.getCfgs().size();

  for (size_t i = 0; i < _numOfServers; i++) {
    _vservers.push_back(VServer(cfg.getCfgs()[i]));
  }
  _defaultCfg = false;
}

// The main routine for setting up the servers listed in the Config.
//
// In the default case there is only one server with default values (like
// serving from the current dir any index.html or whatever,
// server_name="localhost", port="4284" etc...)
void Webserv::_setupServers()
{
  if (_defaultCfg)
    _initDefaultCfg();

  std::vector<VServer>::iterator it = _vservers.begin();
  while (it != _vservers.end()) {
    _setupSingleServer(*it);
    if (it->getSetupFailed()) {
      it = _vservers.erase(it);
      --_numOfServers;
    }
    else
      ++it;
  }
  Logger::log_dbg2("Number of not-failed Servers left after cleanup: " +
      int2str(_numOfServers));
  if (_numOfServers == 0) {
    Logger::log_err("Could not setup any Server!");
    shutdownWebserv();
  }
}

// here one server is being setup, meaning, the `init()` of a server is called
// which is only responsible for binding a socket to a port and start listening
// on it. The rest of the server initialization is being done in... here too?!
// TODO: handle possible exceptions!!!
//
// FIXME: refactor this to handle virtual servers only differing by serverName.
// I.E.: The serverFdMap has to be a multimap! meaning: we can have multiple
// servers per fd only differing by name. But this i will first have to
// implement in VServer initialization.
void Webserv::_setupSingleServer(VServer& srv)
{
  Logger::log_msg("Setting up this server:");
  try {
    srv.init();
    for (std::set<int>::const_iterator it = srv.getListenFds().begin();
        it != srv.getListenFds().end();
        it++)
      _vserverFdMap[*it].push_back(&srv);
  } catch (const VServer::ServerInitException& e) {
    Logger::log_err(
        "Caught exception while trying to init srv " + srv.getServerName());
    Logger::log_err(e.what());
    srv.setSetupFailed();
  }
  srv.printCfg();
}

// nothing to do here so far...
void Webserv::_shutdownAllServers()
{
  Logger::log_msg("Bye-bye from m0fr1m's webserv!");
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
  _epoll.setup(_vservers, _numOfServers);

  // the main loop
  while (_shutdown_server == false) {
    int nfds = _epoll.wait();
    if (nfds == -1 && errno != EINTR)
      throw(WebservRunException("epoll_wait failed"));

    for (int eventIdx = 0; eventIdx < nfds; ++eventIdx) {
      Logger::log_dbg1("Num of nfds from epoll: " + int2str(nfds));
      int currentFd = _epoll.getEventFd(eventIdx);

      // 1) new connection
      //
      // FIXME: it is possible for us to have servers with different server
      // names listening on the same hostname:port connection. will this work
      // for them too?
      // ANSWER: Yes! But the selection process will become a little more
      // difficult. Meaning: _getServerByFd will return a list of servers. and
      // the request will have to be parsed here in order to find out the
      // server_name
      // There is 1 event for the connect. And another one, EPOLLIN, which is
      // the write/send which contains the request. So in order to handle
      // Virtual Servers we only have to return the list of all Servers
      // listening on a fd the client is connected to. In the request handling
      // part we can then route the request to the correct vsrv.
      if (_isServerFd(currentFd) && _numOfClients < MAX_CLIENTS) {
        std::vector<VServer *> vsrvs = _getServerByFd(currentFd);

        if (vsrvs.empty()) {
          Logger::log_warn("Webserv::run: _getServerByFd returned no servers");
          continue;
        }

        // FIXME: FROM HERE i will have to start refactoring:
        //
        // 1) Clients must be creatable on their own!
        // 2) They also need to store which server(s) they belong to
        // 3) Eventually, after their first processed request it will be clear
        //    which exact server is theirs. Either achieve this by setting a
        //    single server field or reducing servers-vector to one.

        Client *cli;
        if (vsrvs.size() == 1)
          cli = vsrvs[0]->addClient(currentFd);
        else {
          if ((cli = Client::newCliServerless(currentFd)) == NULL) {
            Logger::log_warn(
                "Webserv::run: Client::newCliServerless returned NULL");
            continue;
          }
          cli->setPotentialVsrvs(vsrvs);
        }
        _fdClientMap[cli->getFd()] = cli;
        _epoll.addClient(cli->getFd());
        _numOfClients++;
      }

      // 2) existing connection
      //
      // QUESTION: what if _numOfClients >= MAX_CLIENTS ?!?! we need to handle
      // this!!! Also:
      // FIXME: clarify how we handle connection keepalive with clients
      else {
        Client  *cli  = _fdClientMap[currentFd];
        VServer *vsrv = cli->getVsrv();
        int      evHandlerReturn;

        if (vsrv == NULL) {
          // handle serverName detection... somehow ?!??
          evHandlerReturn = REQ_ERR;
        }
        else {
          if (LOGLEVEL == BRUTAL)
            vsrv->printClients();
          evHandlerReturn = vsrv->handleEvent(_epoll.getEvent(eventIdx), cli);
        }

        if (evHandlerReturn == REQ_ERR || evHandlerReturn == REQ_DISCO) {
          _epoll.removeClient(currentFd);
          if (vsrv)
            vsrv->deleteClient(currentFd);
          _numOfClients--;
          _fdClientMap.erase(currentFd);
        }
        else if (evHandlerReturn == REQ_WRITE)
          _epoll.modifyClient(currentFd, EPOLLOUT);
        else if (evHandlerReturn == REQ_INC || evHandlerReturn == REQ_READ)
          _epoll.modifyClient(currentFd, EPOLLIN);
      }
      _timeoutClients();
    }
  }
  _epoll.closeEpollFd();
}

// For serverless clients there can only be a EPOLLIN event bc it is the first
// real request that is processed
int Webserv::_handleEventServerless(const struct epoll_event& ev, Client *cli)
{
  if (ev.events & EPOLLIN) {
    RequestHandler reqHandler(NULL);
    return reqHandler.readRequest(cli);
  }
  Logger::log_err("Webserv::handleEventServerless: srvless event not EPOLLIN!");
  return REQ_ERR;
}

// TODO: add special treatment for CGI clients here
void Webserv::_timeoutClients()
{
  time_t now = time(NULL);

  std::map< int, Client * >::iterator it = _fdClientMap.begin();
  while (it != _fdClientMap.end()) {
    Client  *cli  = it->second;
    VServer *vsrv = cli->getVsrv();
    if (difftime(now, cli->getLastAccess()) > WsrvLib::WsrvSettings.reqTimeout)
    {
      Logger::log_srv("Timeout",
          "disconnecting cli " + cli->getAddr() + ":" +
              int2str(cli->getPort()) + " due to timeout.");
      _epoll.removeClient(it->first);
      if (vsrv)
        vsrv->deleteClient(it->first);
      _numOfClients--;
      it = eraseIt(_fdClientMap, it);
      continue;
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
