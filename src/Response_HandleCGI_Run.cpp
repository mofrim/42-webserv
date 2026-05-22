/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response_HandleCGI_Run.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 10:36:30 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/22 15:31:25 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "utils.hpp"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

e_HTTPStatus Response::_cgiSetup(std::map<str, str> cgiParams)
{
  str cgiExec   = cgiParams["EXEC"];
  str cgiScript = cgiParams["SCRIPT_FILENAME"];

  _cgiWriteBodySize = _req->getBodySize();

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
    if (argvSplit.size() > 1)
      cgiExec = argvSplit[0];
    argvSplit.push_back(cgiScript);

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
  if (_cgiWriteBodySize == 0) {
    Logger::logDbg2("Response::cgiWrite", "No body to write!");
    _cli->setState(CLI_CGIREAD);
    _cli->delCgiFromEpoll(_cgiParentWriteFd);
    close(_cgiParentWriteFd);
  }

  Logger::logDbg2("(Response::cgiWrite) writing body (" +
      int2str(_cgiBytesWritten) + "/" + int2str(_cgiWriteBodySize) + "/" +
      int2str(_req->getBodySize()) + "):\n" +
      data2hexStr(_req->getBodyRawData(_cgiBytesWritten),
          _cgiWriteBodySize - _cgiBytesWritten));

  ssize_t bytesWritten = write(_cgiParentWriteFd,
      _req->getBodyRawData(_cgiBytesWritten),
      _cgiWriteBodySize - _cgiBytesWritten);

  if (bytesWritten >= 0) {
    if (static_cast<size_t>(bytesWritten) == _cgiWriteBodySize) {
      Logger::logDbg2("Response::cgiWrite", "Done writing body to pipe!");
      _cli->setState(CLI_CGIREAD);
      _cli->delCgiFromEpoll(_cgiParentWriteFd);
      close(_cgiParentWriteFd);
    }
  }
  else {
    Logger::logWarn("Response::cgiWrite", "Failed to write body!");
    _cli->setState(CLI_CGIKO);
    _status = HTTP_502;
  }
}

// this function is responsible for reaping the child process and reporting back
// the exit status or any error
bool Response::cgiEvalChildState()
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
      Logger::logDbg2("Response::cgiEvalChildState", "Child not yet done...");
      break;
    case -1:
      Logger::logDbg1("Response::cgiEvalChildState",
          "waitpid -> -1, errno: " + getErrnoStr());
      if (errno == EAGAIN)
        break;
      // FIXME is this correct in any case???
      else if (errno == ECHILD) {
        Logger::logDbg1("Response::cgiEvalChildState",
            "waitpid -> -1, errno: " + getErrnoStr());
        _cli->setState(CLI_CGICDONE);
        break;
      }
      else {
        Logger::logDbg1("Response::cgiEvalChildState",
            "waitpid -> -1, errno: " + getErrnoStr());
        _status = HTTP_500;
        _cli->setState(CLI_CGIKO);
        return KO;
      }
      break;
    default:
      Logger::logDbg1("Response::cgiEvalChildState", "Child done!");
      if ((WIFEXITED(status) && WEXITSTATUS(status) != 0) || !WIFEXITED(status))
      {
        if (WIFEXITED(status))
          Logger::logDbg1("Response::cgiEvalChildState",
              "execve() failed with status " + int2str(WEXITSTATUS(status)));

        if (WIFSIGNALED(status))
          Logger::logDbg1("Response::cgiEvalChildState",
              "execve() signaled with signal " + int2str(WTERMSIG(status)));

        _status = HTTP_502;
        _cli->setState(CLI_CGIKO);
        return KO;
      }
      else
        _cli->setState(CLI_CGICDONE);
  }
  return OK;
}

void Response::cgiRead()
{
  ssize_t bytesRead = 0;

  bytesRead = read(_cgiParentReadFd, _cgiReadBuffer, CGI_READBUFSIZE);

  Logger::logDbg2("Response::cgiRead", "bytesRead = " + int2str(bytesRead));

  if (bytesRead < 0)
    return;

  if (bytesRead == 0) {
    Logger::logDbg1("Response::cgiRead",
        "got EOF -> done! Read " + int2str(_cgiBody.size()) + " bytes");
    _cgiBody.append(_cgiReadBuffer, bytesRead);
    _cli->setState(CLI_CGIOK);
    _cli->delCgiFromEpoll(_cgiParentReadFd);
    close(_cgiParentReadFd);
    return;
  }

  if (_cgiBody.size() + bytesRead > MAX_CGI_BODY_LENGTH) {
    Logger::logSrv(
        _vsrvName, "CGI Body exceeding MAX_CGI_BODY_LENGTH. Baaad CGI!");
    _status = HTTP_502;
    _cli->setState(CLI_CGIKO);
    _cli->delCgiFromEpoll(_cgiParentReadFd);
    close(_cgiParentReadFd);
  }

  _cgiBody.append(_cgiReadBuffer, bytesRead);
}

void Response::cgiProcessBody()
{
  Logger::logDbg2("Response::cgiProcessBody",
      "BODY FROM SCRIPT:\n" + data2hexStr(_cgiBody.data(), _cgiBody.size()));

  size_t crlfx2 = _cgiBody.find(CRLFX2);

  if (crlfx2 == str::npos) {
    _status = HTTP_502;
    return;
  }

  // we have got a valid header!

  str              headers  = _cgiBody.substr(0, crlfx2 + 2);
  std::vector<str> hdrLines = splitString(headers, CRLF);

  if (hdrLines.size() == 0) {
    _status = HTTP_502;
    return;
  }

  for (size_t i = 0; i < hdrLines.size(); ++i) {
    str&   line = hdrLines[i];
    size_t pos  = line.find(":");
    if (pos != std::string::npos && pos + 1 < line.size()) {
      str key            = strip(line.substr(0, pos));
      str value          = strip(line.substr(pos + 1));
      _respoHeaders[key] = value;
    }
    else {
      _status = HTTP_502;
      return;
    }
  }

  // RFC says: content-type is a MUST!
  if (!_respoHdrHas("Content-Type")) {
    _status = HTTP_502;
    return;
  }

  // PERF this is mem mgmt technically a bit painful to do. it'd be better to
  // just pass on cgiBody as a pointer here. But this is left for future
  // optimization!
  size_t bodySize = _cgiBody.size() - headers.size() - 2;
  _req->setBodySize(bodySize);
  _body.assign(_cgiBody.substr(crlfx2).data() + 4, bodySize);
  _status = HTTP_200;
}

void Response::cgiCleanupFds()
{
  _cli->delCgiFromEpoll(_cgiParentWriteFd);
  _cli->delCgiFromEpoll(_cgiParentReadFd);
  close(_cgiParentWriteFd);
  close(_cgiParentReadFd);
}

// well, the correct solution here would be to register the clients pid to epoll
// using something like pidfd_open syscall to create a epoll-monitorable object
// from the child's pid. but this is not included in the allowed functions! for
// more background:
//
// https://unixism.net/2021/02/making-signals-less-painful-under-linux/
//
// ... and of course pidfd_open(2) ;)
//
void Response::cgiKillProcess()
{
  int wret = waitpid(_cgiPid, 0, WNOHANG);
  int kret = 0;
  if (wret == 0) {
    Logger::logDbg1(
        "Response::cgiKillProcess", "Trying to kill child with SIGTERM...");
    kret = kill(_cgiPid, SIGTERM);
    if (kret == -1 && errno == ESRCH) {
      Logger::logDbg1("Response::cgiKillProcess", "kill == -1, child is gone!");
      return;
    }
  }
  else if (wret == -1) {
    Logger::logDbg1(
        "Response::cgiKillProcess", "waitpid == -1, child is gone!");
    return;
  }
  else {
    Logger::logDbg1("Response::cgiKillProcess", "waitpid > 0, child exited!");
    return;
  }

  // FIXME REMOVEME!
  usleep(500);

  wret = waitpid(_cgiPid, 0, WNOHANG);
  if (wret == 0) {
    Logger::logDbg1(
        "Response::cgiKillProcess", "Trying to kill child with SIGKILL...");
    kret = kill(_cgiPid, SIGKILL);
    if (kret == -1 && errno == ESRCH) {
      Logger::logDbg1("Response::cgiKillProcess", "kill == -1, child is gone!");
      return;
    }
  }
  else if (wret == -1) {
    Logger::logDbg1(
        "Response::cgiKillProcess", "waitpid == -1, child is gone!");
    return;
  }
  else {
    Logger::logDbg1("Response::cgiKillProcess", "waitpid > 0, child exited!");
    return;
  }

  // BUG this is blocking the whole server!!!!
  usleep(1000);

  wret = waitpid(_cgiPid, 0, WNOHANG);
  switch (wret) {
    case 0:
      break;
    case -1:
      Logger::logDbg1(
          "Response::cgiKillProcess", "waitpid == -1, child is gone!");
      return;
    default:
      Logger::logDbg1("Response::cgiKillProcess", "waitpid > 0, child exited!");
      return;
  }

  Logger::logDbg1("Response::cgiKillProcess", "giving up killing CGI process");
}
