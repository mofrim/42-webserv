/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response_HandleCGI_Run.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/17 10:36:30 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/17 11:20:13 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "VServer.hpp"
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
  int parentWriteFd = stdinPipe[1];
  int parentReadFd  = stdoutPipe[0];

  // set parents read write fds non-blocking
  setFdNonBlocking(parentWriteFd);
  setFdNonBlocking(parentReadFd);

  Logger::logBug("CGI: forking " + cgiExec);
  Logger::logBug("CGI: with this script " + cgiScript);

  int status;

  // Logger::logBug("This is the env:");
  // char **p = envp;
  // while (*p) {
  //   Logger::logBug(str(*p));
  //   ++p;
  // }

  // -----------------------------=[ fork! ]=------------------------------ //

  pid_t pid = fork();

  if (pid == -1) {
    Logger::logBug("CGI: fork() failed!");
    close(stdinPipe[0]);
    close(stdinPipe[1]);
    close(stdoutPipe[0]);
    close(stdoutPipe[1]);
    return HTTP_500;
  }

  // the child
  if (pid == 0) {

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

  // |=--------------------------=[ The parent ]=--------------------------=| //

  _cli->setState(CLI_CGIWRITE);

  close(stdinPipe[0]);  // Close unused read end
  close(stdoutPipe[1]); // Close unused write end
  //
  Logger::logBug("Hello from parent!");
  Logger::logBug(
      "body: " + str(_req->getBodyData().begin(), _req->getBodyData().end()));

  if (write(parentWriteFd,
          _req->getBody().getBodyDataAsStr().c_str(),
          _req->getBodyData().size()) == -1)
    return HTTP_500;

  Logger::logBug("...now waiting");

  waitpid(pid, &status, WNOHANG);
  if (WEXITSTATUS(status) != 0) {
    Logger::logBug("CGI: execve() failed!");
    return HTTP_500;
  }

  close(stdinPipe[1]);

  char buffer[READ_BUFSIZE + 1];
  str  body;
  int  ret;

  while ((ret = read(parentReadFd, buffer, READ_BUFSIZE)) > 0) {
    buffer[ret] = '\0';
    body += buffer;
  }
  Logger::logBug("BODY FROM SCRIPT:" + body);
  if (body.find("\r\n\r\n") != std::string::npos) {
    std::string        headers = body.substr(0, body.find("\r\n\r\n"));
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
    body = body.substr(body.find("\r\n\r\n") + 4);
  }
  _body = body;
  close(stdoutPipe[0]);
  return HTTP_200;
}
