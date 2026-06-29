/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/20 12:36:43 by fmaurer           #+#    #+#             */
/*   Updated: 2026/06/29 14:09:37 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include "Logger.hpp"
#include "VServer.hpp"
#include "Webserv.hpp"

#include <cerrno>
#include <unistd.h>
#include <utils.hpp>

// the shutdown flag declared in main.cpp
extern volatile sig_atomic_t g_killme;

Webserv::Webserv(): _defaultCfg(true), _shutdown_server(false), _numOfClients(0)
{}

Webserv::Webserv(const Webserv& other) { (void)other; }

Webserv& Webserv::operator=(const Webserv& other)
{
  (void)other;
  return (*this);
}

// nothing to be done here, so far..
Webserv::~Webserv() {}

Webserv::Webserv(char **envp):
  _defaultCfg(true), _shutdown_server(false), _numOfClients(0), _envp(envp)
{}

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
        "Webserv::readConfig", "Could not parse any vsrvs from cfg!");
    return;
  }

  for (std::vector<VServerCfg>::const_iterator it = cfgs.begin();
      it != cfgs.end();
      ++it)
    _vservers.push_back(VServer(*it));

  Logger::logCfg("WebServ-Timeouts after cfg-parsing: reqTimeout = " +
      int2str(WsrvLib::Settings.reqTimeout) +
      ", cgiTimeout = " + int2str(WsrvLib::Settings.cgiTimeout));
}

// The main routine for setting up the servers listed in the Config.
bool Webserv::_setupServers()
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
  if (_vservers.size() == 0) {
    Logger::logErr("Could not setup any Server!");
    return KO;
  }
  return OK;
}

void Webserv::_setupSingleServer(std::vector<VServer>::iterator srvIt)
{
  Logger::logMsg("Trying to setup server '" + srvIt->getName() + "':");
  try {
    // init() needs the _vservers.begin and srvIt
    srvIt->init(_vservers.begin(), srvIt, _envp);
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

// the logic here is simple: if we are getting an I/O event on one of our
// servers listening sockets (aka ports) this is a connection request ->
// add a new client!
//
// EINTR == epoll_wait was interrupted by signal_handler being called (expl:
// epoll_wait is a syscall and as such atomic. so if any interupt is being
// called the syscall exits)
void Webserv::run()
{
  if (_setupServers() == KO)
    return;

  _epoll.setup(_vservers);

  // the main loop
  while (g_killme == 0) {
    int nfds = _epoll.wait();
    if (nfds == -1 && errno != EINTR)
      throw WebservRunException("epoll_wait failed");

    if (LOGLEVEL == LOG_BRUTAL)
      _epoll.printReadylist();

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
          cli = vsrvs[0]->addClient(this, currentFd);
        else {
          if ((cli = Client::newVirtualCli(this, currentFd)) == NULL) {
            Logger::logWarn(
                "Webserv::run", "Client::newVirtualCli returned NULL");
            continue;
          }
          cli->setPotentialVsrvs(vsrvs);
        }
        _fdClientMap[cli->getFd()] = cli;
        _epoll.addClient(cli->getFd());
        _numOfClients++;
      }

      // 2) existing connection
      else {

        // there can be racing conditions that may lead to ending up with a
        // already deleted Client in the ready-list. here, we make very sure,
        // that we never deref NULL in that case!
        if (_fdClientMap.find(currentFd) == _fdClientMap.end() ||
            _fdClientMap[currentFd] == NULL)
          continue;

        Client *cli = _fdClientMap[currentFd];
        u32     ev  = _epoll.getEvent(eventIdx);

        // skip cli if event is EPOLLOUT but the client has got nothing to send
        // or is still reading a request
        if ((cli->isIdling() || cli->isReading()) && (ev & EPOLLOUT))
          continue;

        if (cli->isCGIing()) {
          if (currentFd == cli->getFd() && ev & (EPOLLIN | EPOLLERR | EPOLLHUP))
            cli->handleEvent(ev);
          else
            cli->handleEventCGI(ev, currentFd);
        }
        else
          cli->handleEvent(ev);

        if (cli->getState() == CLI_DISCO_CGI)
          _handleDiscoCGI(cli, cli->getVsrv());
        else if (cli->isDisco()) {
          _epoll.removeClient(currentFd);
          if (cli->getVsrv())
            cli->getVsrv()->deleteClient(currentFd);
          else
            delete cli;
          _numOfClients--;
          _fdClientMap.erase(currentFd);
        }
        else if (cli->isSending())
          _epoll.modifyClient(currentFd, EPOLLOUT);
        else if (cli->isReading() || cli->isIdling() || cli->isDraining())
          _epoll.modifyClient(currentFd, EPOLLIN);
        else if (cli->getState() == CLI_CGIOK || cli->getState() == CLI_CGIKO) {
          cli->setState(CLI_SEND);
          _epoll.modifyClient(cli->getFd(), EPOLLOUT);
        }
      }
    }
    _timeoutClients();
    if (processCgiKillQueue() == OK || nfds > 0)
      Logger::drawCycleSep();
  }
  _epoll.closeEpollFd();
}

void Webserv::_timeoutClients()
{
  time_t now = time(NULL);

  std::map<int, Client *>::iterator it = _fdClientMap.begin();

  while (it != _fdClientMap.end()) {
    Client *cli = it->second;
    Logger::logDbg2("Timeout",
        "Client " + cli->getIfaceFdStr() +
            " difftime = " + int2str(difftime(now, cli->getLastActive())));
    if (cli->getState() == CLI_CGIREAD || cli->getState() == CLI_CGIRW) {
      Logger::logDbg2(
          "checking cgi client timeout: " + it->second->getIfaceFdStr() + "\n" +
          "difftime: " + int2str(difftime(now, cli->getLastActive())));
      if (difftime(now, cli->getLastActive()) > WsrvLib::Settings.cgiTimeout) {
        Logger::logDbg0("Timing out CGI client..." + cli->getIfaceFdStr());
        cli->timeout();
        _epoll.modifyClient(cli->getFd(), EPOLLOUT);
        cli->setState(CLI_SEND);
        // important to do this right here! otherwise the cgi-fds will remain in
        // the epoll list even though the client is disco
        cli->getReq().getRespo().cgiCleanupFds();
        cli->getReq().setStatusCode(HTTP_504);
      }
    }
    else if (difftime(now, cli->getLastActive()) > WsrvLib::Settings.reqTimeout)
    {
      Logger::logDbg1("Timing out normal client..." + cli->getIfaceFdStr());
      cli->timeout();
      _epoll.modifyClient(cli->getFd(), EPOLLOUT);
      cli->setState(CLI_SEND);
    }
    ++it;
  }
}

// -----------------------------=[ Exceptions ]=----------------------------- //

Webserv::WebservInitException::WebservInitException(const std::string& msg):
  std::runtime_error("WebservInitException: " + msg)
{}

Webserv::WebservRunException::WebservRunException(const std::string& msg):
  std::runtime_error("WebservRunException: " + msg)
{}

// --------------------------------=[ CGI ]=-------------------------------- //

void Webserv::addCgiCliToEpoll(Client *cli, int fdWrite, int fdRead)
{
  _fdClientMap[fdWrite] = cli;
  _fdClientMap[fdRead]  = cli;
  _epoll.addClient(fdWrite, EPOLLOUT);
  _epoll.addClient(fdRead, EPOLLIN);
}

void Webserv::removeCgiFdFromEpoll(int fd)
{
  if (_fdClientMap.find(fd) != _fdClientMap.end()) {
    _fdClientMap.erase(fd);
    _epoll.removeClient(fd);
  }
}

// what is to be done in here?
void Webserv::_handleDiscoCGI(Client *cli, VServer *vsrv)
{
  Logger::logSrv(vsrv->getName(), "Handling CGI Disco!");
  Response& respo = cli->getReq().getRespo();
  respo.cgiCleanupFds();

  // if child process is still running add to killQueue
  if (waitpid(respo.cgiGetCpid(), 0, WNOHANG) == 0)
    addCgiPidToKillQueue(respo.cgiGetCpid());

  int cliFd = cli->getFd();

  // this order is very important!
  _epoll.removeClient(cliFd);
  vsrv->deleteClient(cliFd);

  _fdClientMap.erase(cliFd);
  _numOfClients--;
}

// add a CGI pid to webserv's killQueue. this makes monitoring the kill process
// via epoll cycles possible
void Webserv::addCgiPidToKillQueue(pid_t pid) { _killQueue[pid] = KILL_TERM; }

// process the kill queue. Meaning we skip through the killQueue and send the
// signal that corresponds to the entry's state. in the next epoll-cycle the
// pid's status will be checked, if killing was successful we are good and
// remove the pid from the queue. we also remove the pid from queue if killing
// fails.
//
// this solution even gives meaning to have an epoll-timeout other than -1.
//
// Sending using `kill(-pid, SIGNAL)` here because we want to kill the whole
// process-group belonging to the child process.
bool Webserv::processCgiKillQueue()
{
  if (_killQueue.empty())
    return KO;

  std::map<pid_t, e_KillState>::iterator it = _killQueue.begin();

  while (it != _killQueue.end()) {
    pid_t       pid   = it->first;
    e_KillState state = it->second;
    int         kret  = 0;

    int wret = waitpid(pid, 0, WNOHANG);

    if (wret != 0) {
      Logger::logDbg1("Webserv::processKillQueue",
          "pid " + int2str(pid) + " gone or exited (" + int2str(wret) +
              ") -> removing from _killQueue");
      it = eraseIt(_killQueue, it);
      continue;
    }

    if (state == KILL_TERM) {
      Logger::logDbg1("Webserv::processKillQueue",
          "Sending SIGTERM to pid " + int2str(pid));
      kret       = kill(-pid, SIGTERM);
      it->second = KILL_KILL;
    }
    else {
      Logger::logDbg1("Webserv::processKillQueue",
          "Sending SIGKILL to pid " + int2str(pid));
      kret = kill(-pid, SIGKILL);
    }

    if (kret < 0) {
      Logger::logDbg1("Webserv::processKillQueue",
          "killing pid " + int2str(pid) +
              " failed -> removing from _killQueue");
      it = eraseIt(_killQueue, it);
      continue;
    }

    ++it;
  }
  return OK;
}
