/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response_HandleCGI.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/16 14:54:57 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/26 13:26:04 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "VServer.hpp"
#include "utils.hpp"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

e_HTTPStatus Response::handleCGI(Request& req)
{
  _setFieldsFromReq(req);

  // evaluate the script path and try to find the real fspath to the script
  // file. if everything is fine cgiParams will already hold some env vars we
  // can use in cgiSetup. If it fails _status will be set to some error status.
  std::map<str, str> cgiParams = _cgiEvalScriptPath();
  if (cgiParams.empty())
    return _status;

  // yes! if we made it here we are kind of good to go!

  return this->_cgiSetup(cgiParams);
}

// extracted Routine for handling script files that somehow cannot be accessed
void Response::_cgiHandleBadScript(constr& s)
{
  if (access(s.c_str(), R_OK) == -1 && errno == EACCES)
    _status = HTTP_403;
  else
    _status = HTTP_404;
  _body = WsrvLib::getDefaultStatusPage(_status);
  Logger::logDbg1("CGI script " + s + " access " + getErrnoStr() + " -> " +
      int2str(_status));
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

  Logger::logDbg1("Response::_cgiDetermineScriptFile",
      "Trying this script path: " + cgiScript);

  ft = getFileType(cgiScript);
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

  Logger::logDbg1("Response::_cgiDetermineScriptFile",
      "Trying this script path: " + cgiScript);

  cgiScript = r.getRoot() + r.getPath() + firstSeg;
  ft        = getFileType(cgiScript);
  if (ft == 0)
    return std::make_pair(cgiScript, 2);
  else if (ft < 0)
    return std::make_pair(cgiScript, -1);

  cgiScript = r.getRoot() + r.getPath() + _targetPath;

  Logger::logDbg1("Response::_cgiDetermineScriptFile",
      "Trying this script path: " + cgiScript);

  ft = getFileType(cgiScript);
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
  env["REQUEST_METHOD"]    = meth2str(_req->getMethod());
  env["SERVER_NAME"]       = _req->getHost();
  env["SERVER_PORT"]       = int2str(_req->getHostPort());
  env["SERVER_PROTOCOL"] = WsrvLib::httpVer2Str(_req->getReqline().httpVersion);
  env["SERVER_SOFTWARE"] = "mofrim's WebServ";

  // get PATH from global env...
  // this is surely not optimal from a opssec perspective, but convenient for
  // script execution

  char **globalEnvp = _vsrv->getEnvp();
  str    _globalEnvPATH;
  if (globalEnvp != NULL) {
    char **p = globalEnvp;
    while (*p) {
      str cur(*p);
      if (cur.length() > 4 && !cur.compare(0, 4, "PATH"))
        _globalEnvPATH = cur;
      ++p;
    }
  }

  char **envp = new char *[env.size() + !_globalEnvPATH.empty() + 1];
  if (!envp)
    throw std::runtime_error("(Response::_cgiBuildEnv) allocation failed!");

  size_t envIdx = 0;

  for (std::map<str, str>::const_iterator it = env.begin(); it != env.end();
      ++it, ++envIdx)
  {
    str itmString = it->first + "=" + it->second;

    // strcpy()
    //
    //     These  functions  copy the string pointed to by src, into a string at
    //     the buffer pointed to by dst.  The programmer is responsible for
    //     allocating  a  destination buffer  large  enough, that is,
    //     strlen(src) + 1... -> NUL-byte!!!
    envp[envIdx] = new char[itmString.size() + 1];
    if (!envp[envIdx])
      throw std::runtime_error("(Response::_cgiBuildEnv) allocation failed!");
    strcpy(envp[envIdx], itmString.c_str());
  }
  if (!_globalEnvPATH.empty()) {
    envp[envIdx] = new char[_globalEnvPATH.size() + 1];
    strcpy(envp[envIdx], _globalEnvPATH.c_str());
    ++envIdx;
  }
  envp[envIdx] = NULL;

  return envp;
}

bool Response::cgiIsWriteFd(int fd) const { return fd == _cgiParentWriteFd; }

bool Response::cgiIsRead(int fd) const { return fd == _cgiParentReadFd; }

pid_t Response::cgiGetCpid() const { return _cgiPid; }
