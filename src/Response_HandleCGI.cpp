/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response_HandleCGI.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/16 14:54:57 by fmaurer           #+#    #+#             */
/*   Updated: 2026/06/30 12:34:34 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "Logger.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "VServer.hpp"
#include "utils.hpp"

#include <algorithm>
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
    // normally this should be 500 as i see it, but the 42tester thinks
    // differently about this!
    _status = HTTP_403;
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
  str                 cgiScriptRelativePath;

  Route& r = *_matchedRoute;

  // try every possible path for finding the script file. as all of theses paths
  // should be at least accessible a failure here means general failure.
  detRet = _cgiDetermineScriptFile();
  if (detRet.second == -1) {
    _cgiHandleBadScript(detRet.first);
    return cgiParams;
  }

  cgiScriptRelativePath = detRet.first;

  Logger::logDbg1("Found this cgiScript now: " + cgiScriptRelativePath);

  // ok! good so far! now, check if extension is supported!

  size_t dotPos = cgiScriptRelativePath.rfind(".");
  if (dotPos == str::npos || dotPos == cgiScriptRelativePath.length() - 1) {
    Logger::logSrv(_vsrvName, "No file-ext found in requested CGI script!");
    _status = HTTP_403;
    _body   = WsrvLib::getDefaultStatusPage(_status);
    return cgiParams;
  }
  str ext = cgiScriptRelativePath.substr(dotPos + 1);

  std::map<str, str>::const_iterator extIt = r.getCgi().find(ext);
  if (extIt == r.getCgi().end()) {
    Logger::logSrv(_vsrvName, "CGI file-ext not supported!");
    _status = HTTP_403;
    _body   = WsrvLib::getDefaultStatusPage(_status);
    return cgiParams;
  }

  cgiParams["EXEC"] = extIt->second;

  // last but not least we check if we can read that thing at all!

  if (access(cgiScriptRelativePath.c_str(), R_OK) == -1) {
    Logger::logSrv(_vsrvName, "Cannot read CGI script!");
    _status = HTTP_403;
    _body   = WsrvLib::getDefaultStatusPage(_status);
    return cgiParams;
  }

  // and we're good to go! build first CGI env params from local vars
  _cgiBuildEnvPathsAndURI(cgiParams, cgiScriptRelativePath);

  return cgiParams;
}

void Response::_cgiBuildEnvPathsAndURI(
    std::map<str, str>& cgiParams, constr& cgiScriptRelativePath)
{
  size_t lastSeg = cgiScriptRelativePath.rfind('/');
  str    scriptFilename;
  if (lastSeg != str::npos && lastSeg + 1 < cgiScriptRelativePath.size())
    scriptFilename = cgiScriptRelativePath.substr(lastSeg + 1);
  else
    scriptFilename = cgiScriptRelativePath;

  // parse SCRIPT_NAME env var from found script file
  str    targetURI = _reqline.target.getPath();
  size_t scriptPos = targetURI.find(scriptFilename);
  if (scriptPos != str::npos) {
    cgiParams["SCRIPT_NAME"] =
        targetURI.substr(0, scriptPos + scriptFilename.length());

    // PATH_INFO should be everything path-like after the scriptFilename until
    // the query string (? in our case). in our case this is everything after
    // the scriptFilename in the targetURI
    cgiParams["PATH_INFO"] =
        targetURI.substr(scriptPos + scriptFilename.length());
    if (cgiParams["PATH_INFO"].empty())
      cgiParams["PATH_INFO"] = "/";
  }
  else {
    cgiParams["SCRIPT_NAME"] = scriptFilename;
    cgiParams["PATH_INFO"]   = "/";
  }
}

// Build the env for the CGI to run. Returns the env for passing it to execve
// but also extends cgiParams param map with the new vars.
//
// Main resource: https://www.rfc-editor.org/rfc/rfc3875#page-10
char **Response::_cgiBuildEnv(std::map<str, str>& cgiParams)
{
  std::map<str, str>& env = cgiParams;

  env["REQUEST_URI"] = _reqline.target.getStr();

  // INFO: did not find any other way to satisfy the 42tester. In principle this
  // is incorrect!
  env["PATH_INFO"] = env["REQUEST_URI"];

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

  // add all headers from request as "HTTP_HEADER_NAME=HEADER_VALUE" pairs to
  // env
  for (std::map<str, str>::iterator it = _req->getHeaders().begin();
      it != _req->getHeaders().end();
      ++it)
  {
    str env_name = "HTTP_" + it->first;
    std::replace(env_name.begin(), env_name.end(), '-', '_');
    std::transform(
        env_name.begin(), env_name.end(), env_name.begin(), ::toupper);
    env[env_name] = it->second;
  }

  // get PATH & PWD from global env...
  // this is surely not optimal from a opssec perspective, but convenient for
  // script execution
  char **globalEnvp = _vsrv->getEnvp();
  str    _globalEnvPATH;
  str    _pwd;
  if (globalEnvp != NULL) {
    char **p = globalEnvp;
    while (*p) {
      str cur(*p);
      if (cur.length() > 4 && !cur.compare(0, 4, "PATH"))
        _globalEnvPATH = cur;
      if (cur.length() >= 3 && !cur.compare(0, 3, "PWD"))
        _pwd = cur;
      ++p;
    }
  }

  // now that we have _pwd we can set the PATH_TRANSLATED and the scripts
  // workdir which defaults to matched routes root!
  // definitely a TODO to make this configurable via a `cgiRoot` variable!
  // WARN: we simply ignore here that it might be empty in some very edgy cases!
  URI workdir;
  if (_pwd.find('=') != str::npos &&
      !workdir
          .parsePath(
              _pwd.substr(_pwd.find('=') + 1) + "/" + _matchedRoute->getRoot())
          .empty())
  {
    env["SCRIPT_WORKDIR"]  = workdir.getStr();
    env["SCRIPT_FILENAME"] = env["SCRIPT_WORKDIR"] + env["SCRIPT_NAME"];
    env["PATH_TRANSLATED"] = env["SCRIPT_WORKDIR"] + env["PATH_INFO"];
  }
  else {
    env["SCRIPT_WORKDIR"]  = _pwd;
    env["SCRIPT_FILENAME"] = env["SCRIPT_NAME"];
    env["PATH_TRANSLATED"] = "/";
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
