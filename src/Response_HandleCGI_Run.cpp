/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response_HandleCGI_Run.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 10:36:30 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/29 23:12:15 by fmaurer          ###   ########.fr       */
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

    // make the CGI child process its own process group
    setpgid(0, 0);

    char **envp = _cgiBuildEnv(cgiParams);

    // char **pp = envp;
    // while (*pp) {
    //   std::cout << *pp << std::endl;
    //   pp++;
    // }

    close(stdinPipe[1]);
    close(stdoutPipe[0]);
    dup2(stdinPipe[0], STDIN_FILENO);
    dup2(stdoutPipe[1], STDOUT_FILENO);
    close(stdinPipe[0]);
    close(stdoutPipe[1]);

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

    Logger::logDbg1("Response::cgiWrite", "No body to write!");
    _cli->setState(CLI_CGIREAD);
    _cli->delCgiFromEpoll(_cgiParentWriteFd);
    close(_cgiParentWriteFd);
    return;
  }

  Logger::logDbg2("(Response::cgiWrite) writing body :\n" +
      printDataTrunc(_req->getBodyRawData(_cgiBytesWritten),
          _cgiWriteBodySize - _cgiBytesWritten,
          100));

  ssize_t bytesWritten = write(_cgiParentWriteFd,
      _req->getBodyRawData(_cgiBytesWritten),
      _cgiWriteBodySize - _cgiBytesWritten);

  if (bytesWritten >= 0) {
    _cgiBytesWritten += bytesWritten;
    if (static_cast<size_t>(_cgiBytesWritten) == _cgiWriteBodySize) {

      Logger::logDbg1("Response::cgiWrite", "Done writing body to pipe!");

      _cli->setState(CLI_CGIREAD);
      _cli->delCgiFromEpoll(_cgiParentWriteFd);
      close(_cgiParentWriteFd);
    }
    else
      Logger::logDbg1("Response::cgiWrite",
          "Wrote " + int2str(_cgiBytesWritten) + " of " +
              int2str(_cgiWriteBodySize) + " bytes to child");
  }
  else {
    Logger::logDbg1("Response::cgiWrite", "Failed to write body!");
    _cli->setState(CLI_CGIKO);
    _status = HTTP_502;
  }
}

// this function is responsible for reaping the child process and reporting back
// the exit status or any error
// returns:
//
//   * -1 or CHILD_RUNNING if the child process is still running
//   * 0 - 255, the exit status of a child that exited
//   * 4242 or CHILD_GONE when the child process is gone
//
// so, if we want to check for an error with the child process we can test for
// return > 0.
int Response::cgiEvalChildState()
{

  if (_cli->getState() == CLI_CGICDONE)
    return 0;

  int status  = 0;
  int waitRet = 0;

  // from the man:
  //
  // waitpid():  on success, returns the process ID of the child whose state has
  // changed; if WNOHANG was specified and one or more child(ren) specified by
  // pid exist, but  have  not yet changed state, then 0 is returned.  On
  // failure, -1 is returned.
  waitRet = waitpid(_cgiPid, &status, WNOHANG);

  switch (waitRet) {
    case 0:
      Logger::logDbg2("Response::cgiEvalChildState", "Child not yet done...");
      return CHILD_RUNNING;
    case -1:
      Logger::logDbg1(
          "Response::cgiEvalChildState", "waitpid -> -1, " + getErrnoStr());
      return CHILD_GONE;
    default:
      Logger::logDbg1("Response::cgiEvalChildState", "Child done!");
      if ((WIFEXITED(status) && WEXITSTATUS(status) != 0) || !WIFEXITED(status))
      {
        if (WIFEXITED(status)) {
          Logger::logDbg1("Response::cgiEvalChildState",
              "execve() failed with status " + int2str(WEXITSTATUS(status)));
          return WEXITSTATUS(status);
        }

        if (WIFSIGNALED(status)) {
          Logger::logDbg1("Response::cgiEvalChildState",
              "execve() signaled with signal " + int2str(WTERMSIG(status)));
          return WTERMSIG(status);
        }
      }
  }
  return 0;
}

void Response::cgiRead()
{
  ssize_t bytesRead = 0;

  bytesRead = read(_cgiParentReadFd, _cgiReadBuffer, CGI_READBUFSIZE);

  Logger::logDbg2("Response::cgiRead", "bytesRead = " + int2str(bytesRead));

  if (bytesRead < 0) {
    Logger::logSrv(_vsrvName, "CGI: Reading failed!");
    _status = HTTP_502;
    _cli->setState(CLI_CGIKO);
  }

  if (bytesRead == 0) {
    Logger::logDbg1("Response::cgiRead",
        "got EOF -> done! Read " + int2str(_cgiBody.size()) + " bytes");
    _cgiBody.append(_cgiReadBuffer, bytesRead);
    _cli->setState(CLI_CGIOK);
    return;
  }

  if (_cgiBody.size() + bytesRead > MAX_CGI_BODY_LENGTH) {
    Logger::logSrv(
        _vsrvName, "CGI: cgiBody exceeding MAX_CGI_BODY_LENGTH. Baaad CGI!");
    _status = HTTP_502;
    _cli->setState(CLI_CGIKO);
  }

  _cgiBody.append(_cgiReadBuffer, bytesRead);
}

void Response::cgiProcessBody()
{
  Logger::logDbg2("Response::cgiProcessBody",
      "BODY FROM SCRIPT:\n" +
          printDataTrunc(_cgiBody.data(), _cgiBody.size(), 200));

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

  if (_respoHdrHas("Status")) {
    str          cgiStatus = _respoHeaders["Status"];
    e_HTTPStatus s         = WsrvLib::str2HttpStatus(cgiStatus);
    Logger::logBug("cgiStatus: " + int2str(s));
    if (s != HTTP_0)
      _status = s;
    _respoHeaders.erase("Status");
  }

  // PERF this is mem mgmt technically a bit painful to do. it'd be better to
  // just pass on cgiBody as a pointer here. But this is left for future
  // optimization!
  size_t bodySize = _cgiBody.size() - headers.size() - 2;
  _req->setBodySize(bodySize);
  _body.assign(_cgiBody.substr(crlfx2).data() + 4, bodySize);
  _status = (_status != HTTP_0 ? _status : HTTP_200);
  // _status = HTTP_200;
}

void Response::cgiCleanupFds()
{
  _cli->delCgiFromEpoll(_cgiParentWriteFd);
  _cli->delCgiFromEpoll(_cgiParentReadFd);
  close(_cgiParentWriteFd);
  close(_cgiParentReadFd);
}
