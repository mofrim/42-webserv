/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response_HandleCGI_Run.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 10:36:30 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/18 12:11:17 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "utils.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

e_HTTPStatus Response::_cgiSetup(std::map<str, str> cgiParams)
{
  str cgiExec   = cgiParams["EXEC"];
  str cgiScript = cgiParams["SCRIPT_FILENAME"];

  Logger::logSrv(_vsrvName, "CGI: Exec=" + cgiExec + ", Script=" + cgiScript);

  cgiParams.erase("EXEC");

  // Names a child-centric: read end of stdinPipe, i.e., stdinPipe[0] will be
  // child's STDIN, the parent will write to stdinPipe[1]. Likewise for
  // stdoutPipe.
  int stdinPipe[2];
  int stdoutPipe[2];

  if (pipe(stdinPipe) == -1 || pipe(stdoutPipe) == -1) {
    Logger::logErr("Response::_cgiSetup", "pipe() failed!");
    return HTTP_500;
  }

  // Just to make it easier to keep track of their use ;)
  _cgiParentWriteFd = stdinPipe[1];
  _cgiParentReadFd  = stdoutPipe[0];

  // set parents read write fds non-blocking
  if (setFdNonBlocking(_cgiParentWriteFd) == -1 ||
      setFdNonBlocking(_cgiParentReadFd) == -1)
  {
    Logger::logErr("Reponse::_cgiSetup", "fcntl() failed!");
    close(stdinPipe[0]), close(stdinPipe[1]), close(stdoutPipe[0]),
        close(stdoutPipe[1]);
    return HTTP_500;
  }

  // -----------------------------=[ fork! ]=------------------------------ //

  _cgiPid = fork();

  if (_cgiPid == -1) {
    Logger::logErr("Reponse::_cgiSetup", "CGI: fork() failed!");
    close(stdinPipe[0]), close(stdinPipe[1]), close(stdoutPipe[0]),
        close(stdoutPipe[1]);
    return HTTP_500;
  }

  // ----------------------------=[ the child ]=---------------------------- //

  if (_cgiPid == 0) {

    close(stdinPipe[1]);
    close(stdoutPipe[0]);
    dup2(stdinPipe[0], STDIN_FILENO);
    dup2(stdoutPipe[1], STDOUT_FILENO);
    close(stdinPipe[0]);
    close(stdoutPipe[1]);

    char **envp = _cgiBuildEnv(cgiParams);

    // handling possible cgiExecs like `/usr/bin/env bash`
    // NOTE: cgiScript is assumed to don't have any whitespaces
    std::vector<str> argvSplit = splitStrWhite(cgiExec);
    if (argvSplit.size() > 1) {
      cgiExec = argvSplit[0];
      argvSplit.push_back(cgiScript);
    }

    char **argv = new char *[argvSplit.size() + 1];
    if (argv == NULL)
      exit(1);
    for (size_t i = 0; i < argvSplit.size(); ++i) {
      argv[i] = new char[argvSplit[i].size() + 1];
      if (argv[i] == NULL)
        exit(1);
      strcpy(argv[i], argvSplit[i].c_str());
    }
    argv[argvSplit.size()] = NULL;

    std::cerr << argv[0] << argv[1] << std::endl;

    // // being almost completely C++ish here ;)
    // char *argv[] = {
    //     new char[cgiExec.length() + 1], new char[cgiScript.length() + 1],
    //     NULL};
    // strcpy(argv[0], cgiExec.c_str());
    // strcpy(argv[1], cgiScript.c_str());

    execve(cgiExec.c_str(), argv, envp);

    char **p = envp;
    while (*p) {
      delete[] *p;
      ++p;
    }
    delete[] envp;

    p = argv;
    while (*p) {
      delete[] *p;
      ++p;
    }
    delete[] argv;

    exit(1);
  }

  // |=--------------------------=[ the parent ]=--------------------------=| //

  _cli->setState(CLI_CGIRW);
  _cli->addCgiToEpoll(_cgiParentWriteFd, _cgiParentReadFd);

  // close ununsed fds
  close(stdinPipe[0]);
  close(stdoutPipe[1]);

  Logger::logDbg1("Response::_cgiSetup", "Done!");

  return HTTP_200;
}

void Response::cgiWrite()
{
  Logger::logDbg2("(Response::cgiWrite) writing body:\n" +
      str(_req->getBodyData().begin(), _req->getBodyData().end()));

  ssize_t bytesWritten = 0;

  bytesWritten = write(_cgiParentWriteFd,
      _req->getBody().getBodyDataAsStr().c_str(),
      _req->getBodyData().size());

  if (bytesWritten >= 0) {

    if (static_cast<size_t>(bytesWritten) == _req->getBodyData().size()) {
      Logger::logDbg2("Response::cgiWrite", "Done writing body to pipe!");
      _cli->setState(CLI_CGIREAD);
      _cli->delCgiFromEpoll(_cgiParentWriteFd);
      close(_cgiParentWriteFd);
    }
    else {
      Logger::logWarn("Response::cgiWrite", "Failed to write complete body!");
      _cli->setState(CLI_CGIKO);
      _status = HTTP_500;
    }
  }
}

bool Response::cgiWait()
{

  if (_cli->getState() == CLI_CGICDONE)
    return OK;

  int status  = 0;
  int waitRet = 0;

  // from the man:
  //
  // waitpid():  on success, returns the process ID of the child whose state has
  // changed; if WNOHANG was specified and one or more child(ren) specified by
  // pid exist, but  have  not yet changed state, then 0 is returned.  On
  // failure, -1 is returned.
  //
  waitRet = waitpid(_cgiPid, &status, WNOHANG);

  switch (waitRet) {
    case 0:
      Logger::logDbg2("Response::cgiWait", "Child not yet done...");
      break;
    case -1:
      Logger::logDbg1(
          "Response::cgiWait", "waitpid -> -1, errno: " + getErrnoStr());
      if (errno == EAGAIN)
        break;
      else if (errno == ECHILD) {
        _cli->setState(CLI_CGICDONE);
        break;
      }
      else {
        _status = HTTP_500;
        _cli->setState(CLI_CGIKO);
        return KO;
      }
      break;
    default:
      Logger::logDbg1("Response::cgiWait", "Child done!");
      if ((WIFEXITED(status) && WEXITSTATUS(status) != 0) || !WIFEXITED(status))
      {
        if (WIFEXITED(status))
          Logger::logDbg1("CGI: execve() failed with status " +
              int2str(WEXITSTATUS(status)));

        if (WIFSIGNALED(status))
          Logger::logDbg1(
              "CGI: execve() failed with status " + int2str(WTERMSIG(status)));

        _status = HTTP_500;
        _cli->setState(CLI_CGIKO);
        return KO;
      }
      else
        _cli->setState(CLI_CGICDONE);
  }
  return OK;
}

// FIXME make this binary-data-proof
void Response::cgiRead()
{
  Logger::logDbg1("Response::cgiRead", "Reading...");
  char    buffer[READ_BUFSIZE + 1];
  ssize_t bytesRead = 0;

  bytesRead = read(_cgiParentReadFd, buffer, READ_BUFSIZE);

  Logger::logDbg1("Response::cgiRead", "bytesRead = " + int2str(bytesRead));

  if (bytesRead < 0)
    return;

  if ((bytesRead == 0 || bytesRead < READ_BUFSIZE) &&
      _cli->getState() == CLI_CGICDONE)
  {
    Logger::logDbg1("Response::cgiRead", "Done! Sending Response...");
    buffer[bytesRead] = '\0';
    _cgiBody += buffer;
    _cli->setState(CLI_CGIOK);
    _cli->delCgiFromEpoll(_cgiParentReadFd);
    close(_cgiParentReadFd);
    return;
  }

  buffer[bytesRead] = '\0';
  _cgiBody += buffer;
}

// FIXME make this a bit more sophisticated!
void Response::cgiProcessBody()
{
  Logger::logDbg2("Response::cgiProcessBody", "BODY FROM SCRIPT:\n" + _cgiBody);

  if (_cgiBody.find(CRLFX2) != std::string::npos) {
    str                headers = _cgiBody.substr(0, _cgiBody.find(CRLFX2));
    std::istringstream iss(headers);
    str                line;
    while (std::getline(iss, line)) {
      size_t pos = line.find(": ");
      if (pos != std::string::npos) {
        str key            = line.substr(0, pos);
        str value          = line.substr(pos + 2);
        _respoHeaders[key] = value;
      }
    }
    _body   = _cgiBody.substr(_cgiBody.find(CRLFX2) + 4);
    _status = HTTP_200;
  }
  else
    _status = HTTP_500;
}

void Response::cgiCleanupFds()
{
  _cli->delCgiFromEpoll(_cgiParentWriteFd);
  _cli->delCgiFromEpoll(_cgiParentReadFd);
  close(_cgiParentWriteFd);
  close(_cgiParentReadFd);
}

void Response::cgiKillProcess()
{
  int wret = waitpid(_cgiPid, 0, WNOHANG);
  if (wret == 0)
    kill(_cgiPid, SIGTERM);
  else if (wret == -1) {
    Logger::logDbg1("Response::cgiKillProcess", "waitpid -1");
    return;
  }
  else {
    Logger::logDbg1("Response::cgiKillProcess", "waitpid > 0");
    return;
  }

  // FIXME REMOVEME!
  usleep(500);

  if (waitpid(_cgiPid, 0, WNOHANG) == _cgiPid) {
    Logger::logDbg1("Response::cgiKillProcess", "killed with SIGTERM");
    return;
  }
  else
    kill(_cgiPid, SIGKILL);

  wret = waitpid(_cgiPid, 0, WNOHANG) == _cgiPid;

  if (wret == _cgiPid) {
    Logger::logDbg1("Response::cgiKillProcess", "killed with SIGKILL");
    return;
  }
  else
    Logger::logDbg1(
        "Response::cgiKillProcess", "giving up killing CGI process");
}
