/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response_HandleCGI.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/16 14:54:57 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/17 02:37:42 by fmaurer          ###   ########.fr       */
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

bool Response::_handleCGI()
{
  // Route& r = *_matchedRoute;

  // first: scriptName, second: interpreter
  std::map<str, str> cgiParams = _cgiEvalScriptPath();
  if (cgiParams.empty())
    return KO;

  // yes! if we made it here we are kind of good to go!

  Logger::logBug("CGI: cgiScript = " + cgiParams["SCRIPT_NAME"]);

  return this->_cgiRun(cgiParams);
}

// extracted Routine for handling script files that somehow cannot be accessed
void Response::_cgiHandleBadScript(constr& s)
{
  if (access(s.c_str(), R_OK) == -1 && errno == EACCES)
    _status = HTTP_403;
  else
    _status = HTTP_404;
  _body = WsrvLib::getDefaultStatusPage(_status);
  Logger::logBug("CGI script " + s + " access errno: " + int2str(errno) + " " +
      strerror(errno) + " -> " + int2str(_status));
}

// the idea is the following:
//
// 1) check if the routes path already was a script
// 2) check if the routes path + the first segment of the _targetPath are the
// script
// 3) lastly check if full targetPath is a file.
//
// the logic: any of these paths should be at least accesible (dirs) if a later
// path is should be successful. SO
std::pair<str, int> Response::_cgiDetermineScriptFile()
{
  Route& r  = *_matchedRoute;
  int    ft = 42;
  str    cgiScript;

  // check if _matchedRoute's path is already the script-file!

  cgiScript = r.getRoot() + r.getPath();
  ft        = getFileType(cgiScript);
  if (ft == 0)
    return std::make_pair(cgiScript, 1);
  else if (ft < 0)
    return std::make_pair(cgiScript, -1);

  str firstSeg;
  if (_targetPath.length() > 1) {
    size_t endSeg = _targetPath.find('/', 1);
    if (endSeg != str::npos)
      firstSeg = _targetPath.substr(0, endSeg);
  }
  cgiScript = r.getRoot() + r.getPath() + firstSeg;
  ft        = getFileType(cgiScript);
  if (ft == 0)
    return std::make_pair(cgiScript, 2);
  else if (ft < 0)
    return std::make_pair(cgiScript, -1);

  cgiScript = r.getRoot() + r.getPath() + _targetPath;
  ft        = getFileType(cgiScript);
  if (ft == 0)
    return std::make_pair(cgiScript, 3);

  // if the last one fails there is no script for us!
  else
    return std::make_pair(cgiScript, -1);
}

std::map<str, str> Response::_cgiEvalScriptPath()
{
  std::map<str, str>  cgiParams;
  std::pair<str, int> detRet;
  str                 cgiScript;

  Route& r = *_matchedRoute;

  // try every possible path for finding the script file. as all of theses paths
  // should be at least accessible a failure here means general failure.
  detRet = _cgiDetermineScriptFile();
  if (detRet.second == -1) {
    _cgiHandleBadScript(detRet.first);
    return cgiParams;
  }

  cgiScript = detRet.first;

  Logger::logDbg1("Found this cgiScript now: " + cgiScript);

  // ok! good so far! now, check if extension is supported!

  size_t dotPos = cgiScript.rfind(".");
  if (dotPos == str::npos || dotPos == cgiScript.length() - 1) {
    Logger::logSrv(_vsrvName, "No file-ext found in requested CGI script!");
    _status = HTTP_403;
    _body   = WsrvLib::getDefaultStatusPage(_status);
    return cgiParams;
  }
  str ext = cgiScript.substr(dotPos + 1);

  std::map<str, str>::const_iterator extIt = r.getCgi().find(ext);
  if (extIt == r.getCgi().end()) {
    Logger::logSrv(_vsrvName, "CGI file-ext not supported!");
    _status = HTTP_403;
    _body   = WsrvLib::getDefaultStatusPage(_status);
    return cgiParams;
  }

  // last but not least we check if we can read that thing at all!

  if (access(cgiScript.c_str(), R_OK) == -1) {
    Logger::logSrv(_vsrvName, "Cannot read from target CGI script!");
    _status = HTTP_403;
    _body   = WsrvLib::getDefaultStatusPage(_status);
    return cgiParams;
  }

  // and we're good to go!

  size_t lastSeg = cgiScript.rfind('/');
  if (lastSeg != str::npos && lastSeg + 1 < cgiScript.size())
    cgiParams["SCRIPT_NAME"] = cgiScript.substr(lastSeg + 1);

  cgiParams["SCRIPT_FILENAME"] = cgiScript;
  cgiParams["EXEC"]            = extIt->second;

  cgiParams["PATH_INFO"] = _targetPath;

  size_t scriptPos = _targetPath.find(cgiParams["SCRIPT_NAME"]);
  str    transPath;
  if (scriptPos != str::npos)
    transPath =
        _targetPath.substr(scriptPos + cgiParams["SCRIPT_NAME"].length());
  cgiParams["PATH_TRANSLATED"] = r.getRoot() + r.getPath() + transPath;

  return cgiParams;
}

bool Response::_cgiRun(std::map<str, str> cgiParams)
{
  str cgiExec   = cgiParams["EXEC"];
  str cgiScript = cgiParams["SCRIPT_FILENAME"];

  cgiParams.erase("EXEC");

  char **envp = _cgiBuildEnv(cgiParams);
  if (!envp)
    throw std::runtime_error("(Response::_cgiBuildEnv) allocation failed!");

  char **p = envp;

  Logger::logBug("Response::_cgiRun", "Build this env:");
  while (*p) {
    Logger::logBug(str(*p));
    p++;
  }

  // Names a child-centric: read end of stdinPipe, i.e., stdinPipe[0] will be
  // child's STDIN, the parent will write to stdinPipe[1]. Likewise for
  // stdoutPipe.
  int stdinPipe[2];
  int stdoutPipe[2];

  if (pipe(stdinPipe) == -1 || pipe(stdoutPipe) == -1) {
    Logger::logBug("CGI: pipe() failed!");
    return KO;
  }

  Logger::logBug("CGI: forking " + cgiExec);
  Logger::logBug("CGI: with this script " + cgiScript);

  int status;

  pid_t pid = fork();

  if (pid == -1) {
    Logger::logBug("CGI: fork() failed!");
    return KO;
  }

  // the child
  if (pid == 0) {

    close(stdinPipe[1]);
    close(stdoutPipe[0]);

    dup2(stdinPipe[0], STDIN_FILENO);   // Redirect stdin
    dup2(stdoutPipe[1], STDOUT_FILENO); // Redirect stdout

    close(stdinPipe[0]);
    close(stdoutPipe[1]);

    char *argv[] = {
        const_cast<char *>(cgiExec.c_str()),
        const_cast<char *>(cgiScript.c_str()),
        NULL};
    execve(cgiExec.c_str(), argv, envp);
    exit(1);
  }
  else if (pid > 0) {
    // Parent process
    close(stdinPipe[0]);  // Close unused read end
    close(stdoutPipe[1]); // Close unused write end
    //
    Logger::logBug("Hello from parent!");
    Logger::logBug(
        "body: " + str(_req->getBodyData().begin(), _req->getBodyData().end()));
    if (write(stdinPipe[1],
            _req->getBody().getBodyDataAsStr().c_str(),
            _req->getBodyData().size()) == -1)
      return KO;
    Logger::logBug("...now waiting");
    waitpid(pid, &status, WNOHANG);
    if (WEXITSTATUS(status) != 0) {
      Logger::logBug("CGI: execve() failed!");
      return KO;
    }
    close(stdinPipe[1]);
    char buffer[READ_BUFSIZE + 1];
    str  body;
    int  ret;
    while ((ret = read(stdoutPipe[0], buffer, READ_BUFSIZE)) > 0) {
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
    return OK;
  }
  return OK;
}

// https://www.rfc-editor.org/rfc/rfc3875#page-10
char **Response::_cgiBuildEnv(std::map<str, str> cgiParams)
{
  std::map<str, str> env = cgiParams;

  env["CONTENT_LENGTH"]    = int2str(_req->getBodySize());
  env["CONTENT_TYPE"]      = _req->getHeaders()["Content-Type"];
  env["GATEWAY_INTERFACE"] = "CGI/1.1";
  env["QUERY_STRING"]      = _req->getReqline().target.getQueryStr();
  env["REMOTE_ADDR"]       = _cli->getAddr();
  env["REMOTE_HOST"]       = _cli->getIfaceFdStr();
  env["REQUEST_METHOD"]    = _req->getMethod();
  env["SERVER_NAME"]       = _req->getHost();
  env["SERVER_PORT"]       = int2str(_req->getHostPort());
  env["SERVER_PROTOCOL"] = WsrvLib::httpVer2Str(_req->getReqline().httpVersion);
  env["SERVER_SOFTWARE"] = "mofrim's WebServ";

  char **globalEnvp    = _vsrv->getEnvp();
  size_t globalEnvSize = 0;
  if (globalEnvp != NULL) {
    char **p = globalEnvp;
    while (*p) {
      ++globalEnvSize;
      ++p;
    }
  }
  char **envp = new char *[env.size() + globalEnvSize + 1];
  if (!envp)
    throw std::runtime_error("(Response::_cgiBuildEnv) allocation failed!");

  size_t envIdx = 0;
  while (envIdx < globalEnvSize) {
    envp[envIdx] = globalEnvp[envIdx];
    envIdx++;
  }

  for (std::map<str, str>::const_iterator it = env.begin();
      it != env.end() && envIdx < env.size() + globalEnvSize;
      ++it, ++envIdx)
  {
    str itmString = it->first + "=" + it->second;

    // strcpy()
    //
    //     These  functions  copy the string pointed to by src, into a string at
    //     the buffer pointed to by dst.  The programmer is responsible for
    //     allocating  a  destination buffer  large  enough, that is,
    //     strlen(src) + 1.
    envp[envIdx] = new char[itmString.size() + 1];
    if (!envp[envIdx])
      throw std::runtime_error("(Response::_cgiBuildEnv) allocation failed!");
    strcpy(envp[envIdx], itmString.c_str());
  }
  envp[envIdx] = NULL;

  return envp;
}
