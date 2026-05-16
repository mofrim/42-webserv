/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response_HandleCGI.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/16 14:54:57 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/16 19:46:06 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "utils.hpp"

#include <cstring>
#include <unistd.h>

void Response::_handleCGI()
{
  // Route& r = *_matchedRoute;

  // first: scriptName, second: interpreter
  std::map<str, str> cgiParams = _cgiEvalScriptPath();
  if (cgiParams.empty())
    return;

  // yes! if we made it here we are kind of good to go!

  Logger::logBug("CGI: cgiScript = " + cgiParams["SCRIPT_NAME"]);

  _body = "CGI!";
}

// extracted Routine for handling script files that somehow cannot be accessed
void Response::_cgiHandleBadScript(constr& s)
{
  if (access(s.c_str(), R_OK) == -1 && errno == EACCES)
    _status = HTTP_403;
  else
    _status = HTTP_404;
  _body = WsrvLib::getDefaultStatusPage(_status);
  Logger::logBug("CGI script " + s + " not found! errno: " + int2str(errno) +
      " " + strerror(errno) + " -> " + int2str(_status));
}

// the idea is the following:
//
// 1) check if the routes path already was a script
// 2) check if the routes path + the first segment of the _targetPath are the
// script
// 3) lastly check if full targetPath is a file.
str Response::_cgiDetermineScriptFile()
{
  Route& r  = *_matchedRoute;
  int    ft = 42;
  str    cgiScript;

  // check if _matchedRoute's path is already the script-file!

  cgiScript = r.getRoot() + r.getPath();
  ft        = getFileType(cgiScript);
  if (ft == 0)
    return cgiScript;

  str firstSeg;
  if (_targetPath.length() > 1) {
    size_t endSeg = _targetPath.find('/', 1);
    if (endSeg != str::npos)
      firstSeg = _targetPath.substr(0, endSeg);
  }
  cgiScript = r.getRoot() + r.getPath() + firstSeg;
  ft        = getFileType(cgiScript);
  if (ft == 0)
    return cgiScript;

  cgiScript = r.getRoot() + r.getPath() + _targetPath;
  ft        = getFileType(cgiScript);
  if (ft == 0)
    return cgiScript;

  return "";
}

std::map<str, str> Response::_cgiEvalScriptPath()
{
  std::map<str, str> cgiParams;
  str                cgiScript;

  Route& r  = *_matchedRoute;
  int    ft = 42;

  // check if _matchedRoute's path is already the script-file!

  cgiScript = _cgiDetermineScriptFile();

  cgiScript = r.getRoot() + r.getPath();
  ft        = getFileType(cgiScript);

  if (ft == 0) {
    cgiParams["SCRIPT_NAME"]     = r.getPath();
    cgiParams["SCRIPT_FILENAME"] = cgiScript;
    cgiParams["PATH_INFO"]       = _targetPath;
  }
  else {
    cgiScript = r.getRoot() + r.getPath() + _targetPath;

    // basic script-file checks
    ft = getFileType(cgiScript);
    if (ft < 0) {
      _cgiHandleBadScript(cgiScript);
      return cgiParams;
    }
    if (ft > 0)
      cgiScript += cgiScript + "/" + r.getIndex();
    ft = getFileType(cgiScript);
    if (ft < 0) {
      _cgiHandleBadScript(cgiScript);
      return cgiParams;
    }
  }

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
    Logger::logSrv(_vsrvName, "Targeted CGI script is read-protected!");
    _status = HTTP_403;
    _body   = WsrvLib::getDefaultStatusPage(_status);
    return cgiParams;
  }
  cgiParams["SCRIPT_FILENAME"] = cgiScript;

  cgiParams["EXEC"] = extIt->second;
  return cgiParams;
}

bool Response::_cgiRun(constr& cgiScript, constr& cgiExec)
{
  (void)cgiScript, (void)cgiExec;
  // int fd[2];
  //
  // if (pipe(fd) == -1) {
  //   Logger::logBug("CGI: pipe() failed!");
  //   return KO;
  // }
  // pid_t pid = fork();
  // if (pid == -1) {
  //   Logger::logBug("CGI: fork() failed!");
  //   return KO;
  // }
  //
  // int status;
  // if (pid == 0) {
  //   if (dup2(fd[0], 0) == -1 || dup2(fd[1], 1) == -1) {
  //     Logger::logBug("CGI: fork() failed!");
  //     return KO;
  //   }
  //   close(fd[1]);
  //   close(fd[0]);
  //   char *argv[] = {
  //       const_cast<char *>(cgiExec.c_str()),
  //       const_cast<char *>(cgiScript.c_str()),
  //       NULL};
  //   execve(cgiExec.c_str(), argv, envp);
  //   exit(1);
  // }
  // else if (pid > 0) {
  //   write(fd[1], req.getRawBody().c_str(), req.getRawBody().size());
  //   waitpid(pid, &status, 0);
  //   if (WEXITSTATUS(status) != 0) {
  //     Console::error("execve failed");
  //     throw ServerException(BadGateway);
  //   }
  //   close(fd[1]);
  //   char        buffer[10240];
  //   std::string body;
  //   int         ret;
  //   while ((ret = read(fd[0], buffer, 1023)) > 0) {
  //     buffer[ret] = '\0';
  //     body += buffer;
  //   }
  //   if (body.find("\r\n\r\n") != std::string::npos) {
  //     std::string        headers = body.substr(0, body.find("\r\n\r\n"));
  //     std::istringstream iss(headers);
  //     std::string        line;
  //     while (std::getline(iss, line)) {
  //       size_t pos = line.find(": ");
  //       if (pos != std::string::npos) {
  //         std::string key            = line.substr(0, pos);
  //         std::string value          = line.substr(pos + 2);
  //         this->responseHeaders[key] = value;
  //       }
  //     }
  //     body = body.substr(body.find("\r\n\r\n") + 4);
  //   }
  //   this->responseBody = body;
  //   close(fd[0]);
  //   return;
  // }
  // else
  //   Console::error("fork failed");
  // throw ServerException(ServerError);

  return true;
}

// https://www.rfc-editor.org/rfc/rfc3875#page-10
str Response::_cgiBuildEnv()
{
  std::map<str, str> env;
  env["AUTH_TYPE"]         = "";
  env["CONTENT_LENGTH"]    = int2str(_req->getBodySize());
  env["CONTENT_TYPE"]      = _req->getHeaders()["Content-Type"];
  env["GATEWAY_INTERFACE"] = "CGI/1.1";

  env["PATH_INFO"]       = ""; // oops! TODO have to change complete URL
  env["PATH_TRANSLATED"] = ""; // parsing for this, fuck!

  env["QUERY_STRING"]    = _req->getReqline().target.getQueryStr();
  env["REMOTE_ADDR"]     = "";
  env["REMOTE_HOST"]     = "";
  env["REMOTE_IDENT"]    = "";
  env["REMOTE_USER"]     = "";
  env["REQUEST_METHOD"]  = "";
  env["SCRIPT_NAME"]     = "";
  env["SERVER_NAME"]     = "";
  env["SERVER_PORT"]     = "";
  env["SERVER_PROTOCOL"] = "";
  env["SERVER_SOFTWARE"] = "";

  // env += str("SERVER_SOFTWARE", "webserv/1.0");
  // env += str("GATEWAY_INTERFACE", "CGI/1.1");
  // env +=
  //     str("REDIRECT_STATUS"
  //         "1");
  // env += str("SERVER_PROTOCOL" req.getVersion());
  // env += str("SERVER_PORT" toString(req.getPort()));
  // env += str("REQUEST_METHOD" req.getMethod());
  // env += str("PATH_INFO" filename);
  // env += str("PATH_TRANSLATED" filename);
  // env += str("QUERY_STRING" getQuery(req.getUri()));
  // env += str("REMOTE_HOST" req.getHost());
  // if (req.getRawBody().size() > 0)
  //   env["CONTENT_LENGTH"] = toString(req.getRawBody().size());
  // if (req.getHeaders()["Content-Type"] != "")
  //   env["CONTENT_TYPE"] = req.getHeaders()["Content-Type"];
  return "";
}
