/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response_HandleCGI_Run.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 10:36:30 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/17 16:06:31 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "utils.hpp"

#include <cstdlib>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

e_HTTPStatus Response::_cgiSetup(std::map<str, str> cgiParams)
{
  str cgiExec   = cgiParams["EXEC"];
  str cgiScript = cgiParams["SCRIPT_FILENAME"];

  cgiParams.erase("EXEC");

  // Names a child-centric: read end of stdinPipe, i.e., stdinPipe[0] will be
  // child's STDIN, the parent will write to stdinPipe[1]. Likewise for
  // stdoutPipe.
  int stdinPipe[2];
  int stdoutPipe[2];

  if (pipe(stdinPipe) == -1 || pipe(stdoutPipe) == -1) {
    Logger::logBug("CGI: pipe() failed!");
    return HTTP_500;
  }

  // Just to make it easier to keep track of their use ;)
  _cgiParentWriteFd = stdinPipe[1];
  _cgiParentReadFd  = stdoutPipe[0];

  // set parents read write fds non-blocking
  if (setFdNonBlocking(_cgiParentWriteFd) == -1 ||
      setFdNonBlocking(_cgiParentReadFd) == -1)
  {
    Logger::logErr("Reponse::_cgiSetup", "CGI: fork() failed!");
    close(stdinPipe[0]), close(stdinPipe[1]), close(stdoutPipe[0]),
        close(stdoutPipe[1]);
    return HTTP_500;
  }

  Logger::logBug("CGI: forking " + cgiExec);
  Logger::logBug("CGI: with this script " + cgiScript);

  // Logger::logBug("This is the env:");
  // char **p = envp;
  // while (*p) {
  //   Logger::logBug(str(*p));
  //   ++p;
  // }

  // -----------------------------=[ fork! ]=------------------------------ //

  _cgiPid = fork();

  if (_cgiPid == -1) {
    Logger::logBug("CGI: fork() failed!");
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
    if (!envp)
      throw std::runtime_error("(Response::_cgiBuildEnv) allocation failed!");

    char *argv[] = {strdup(cgiExec.c_str()), strdup(cgiScript.c_str()), NULL};
    execve(cgiExec.c_str(), argv, envp);
    exit(1);
  }

  // |=--------------------------=[ the parent ]=--------------------------=| //

  _cli->setState(CLI_CGIWRITE);
  _cli->addCgiToEpoll(_cgiParentWriteFd, _cgiParentReadFd);

  // close ununsed fds
  close(stdinPipe[0]);
  close(stdoutPipe[1]);

  return HTTP_200;
}

void Response::cgiWrite()
{
  Logger::logBug("Hello from parent! Trying to write...");
  Logger::logBug(
      "body: " + str(_req->getBodyData().begin(), _req->getBodyData().end()));

  ssize_t bytesWritten = 0;

  bytesWritten = write(_cgiParentWriteFd,
      _req->getBody().getBodyDataAsStr().c_str(),
      _req->getBodyData().size());

  if (bytesWritten >= 0) {
    if (static_cast<size_t>(bytesWritten) == _req->getBodyData().size()) {
      _cli->setState(CLI_CGIWDONE);
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

void Response::cgiWait()
{
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
      Logger::logDbg1("Response::cgiWait", "Child not yet done...");
      if (_cli->getState() == CLI_CGIWDONE)
        _cli->setState(CLI_CGIWAIT);
      else
        _cli->setState(CLI_CGIWRITE);
      break;
    case -1:
      Logger::logDbg1(
          "Response::cgiWait", "waitpid -> -1, errno: " + getErrnoStr());
      if (errno == EAGAIN)
        break;
      else {
        _status = HTTP_500;
        _cli->setState(CLI_CGIKO);
      }
      break;
    default:
      Logger::logDbg1("Response::cgiWait", "Child done!");
      if ((WIFEXITED(status) && WEXITSTATUS(status) != 0) || !WIFEXITED(status))
      {
        Logger::logBug("CGI: execve() failed!");
        _status = HTTP_500;
        _cli->setState(CLI_CGIKO);
      }
      else
        _cli->setState(CLI_CGIREAD);
  }
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

  if (bytesRead == 0 || bytesRead < READ_BUFSIZE) {
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

void Response::cgiProcessBody()
{
  Logger::logBug("Response::cgiProcessBody", "BODY FROM SCRIPT:\n" + _cgiBody);

  if (_cgiBody.find("\r\n\r\n") != std::string::npos) {
    std::string        headers = _cgiBody.substr(0, _cgiBody.find("\r\n\r\n"));
    std::istringstream iss(headers);
    std::string        line;
    while (std::getline(iss, line)) {
      size_t pos = line.find(": ");
      if (pos != std::string::npos) {
        std::string key    = line.substr(0, pos);
        std::string value  = line.substr(pos + 2);
        _respoHeaders[key] = value;
      }
    }
    _body   = _cgiBody.substr(_cgiBody.find("\r\n\r\n") + 4);
    _status = HTTP_200;
  }
  else
    _status = HTTP_500;
}

void Response::cgiShutdown()
{
  _cli->delCgiFromEpoll(_cgiParentWriteFd);
  _cli->delCgiFromEpoll(_cgiParentReadFd);
  close(_cgiParentWriteFd);
  close(_cgiParentReadFd);
}
