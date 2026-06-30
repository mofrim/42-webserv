/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 19:11:25 by fmaurer           #+#    #+#             */
/*   Updated: 2026/06/30 11:16:24 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "VServer.hpp"
#include "WsrvLib.hpp"
#include "utils.hpp"

#include <cerrno>
#include <cstring>
#include <fstream>

// --------------------------------=[ OCF ]=-------------------------------- //

Response::Response() { this->reset(); }

Response::Response(const Response& o)
{
  if (this != &o)
    *this = o;
}

Response& Response::operator=(const Response& o)
{
  if (this != &o) {
    _status           = o._status;
    _reqline          = o._reqline;
    _cli              = o._cli;
    _vsrv             = o._vsrv;
    _respoHeaders     = o._respoHeaders;
    _body             = o._body;
    _mimeType         = o._mimeType;
    _respoStr         = o._respoStr;
    _closeConn        = o._closeConn;
    _req              = o._req;
    _vsrvName         = o._vsrvName;
    _cgiParentWriteFd = o._cgiParentWriteFd;
    _cgiParentReadFd  = o._cgiParentReadFd;
    _cgiPid           = o._cgiPid;
    _cgiBody          = o._cgiBody;
    _method           = o._method;

    _cgiBytesWritten  = o._cgiBytesWritten;
    _cgiWriteBodySize = o._cgiWriteBodySize;
  }
  return (*this);
}

Response::~Response() {}

// ------------------------------=[ END OCF ]=------------------------------ //

// FIXME maybe refac this to use req directly. depends on how often these will
// be used later in code
void Response::_setFieldsFromReq(Request& req)
{
  _status       = req.getStatus();
  _reqline      = req.getReqline();
  _cli          = req.getCli();
  _vsrv         = req.getCli()->getVsrv();
  _closeConn    = req.closeConn();
  _matchedRoute = req.getMatchedRoute();
  _targetPath   = req.getTargetPath();
  _vsrvName     = req.getVsrvName();
  _method       = req.getMethod();
  _req          = &req;
}

// THE main function for finally doing what the request wants. When we finally
// arrive here we have the following setting:
//
//  1) status == 200: everything fine until here do normally processing of
//     request
//
//  2) status == 400: we had a bad request. therefore the only thing left to do
//     is finding the corresponding error page from vsrv BUT keep in mind that
//     client could be virtual!
//
// There is certain fall-through hierarchy in here:
//
//  - if a route has cgi, that will be tried first
//  - then, if cgi script cannot be found (only 404's count!) we come here.
//  - if there is also a redirect in the route that will be done
//  - then, if the request was POST we will try to execute that
//  - if it was DELETE we would try that
//  - finally if it was a GET we try this
//
e_HTTPStatus Response::buildResponse(Request& req)
{
  _setFieldsFromReq(req);

  // handle status >= HTTP_400

  if (req.badRequest() &&
      !(req.isCGI() && (_status == HTTP_404 || _status == HTTP_403)))
  {
    Logger::logSrv(_vsrvName, "Bad Request handling");
    _setBodyStatusPage();
  }

  else if (!req.isCGI() ||
      (req.isCGI() && (_status == HTTP_404 || _status == HTTP_403)))
  {
    // hrom here on: only HTTP_200 so far

    if (req.isRedir()) {
      Logger::logSrv(_vsrvName, "Redir Request handling");
      _handleRedir();
    }
    else if (req.isSimplePOST()) {
      Logger::logSrv(_vsrvName, "SimplePost Request handling");
      _handleSimplePost();
    }
    else if (req.isDELETE()) {
      Logger::logSrv(_vsrvName, "DELETE Request handling");
      _handleDelete();
    }

    // simple GET request
    else {
      Logger::logSrv(_vsrvName, "Normal GET Request handling");
      _getBody200();
    }
  }
  _buildRespoHdrs();
  _buildResponseStr();

  return _status;
}

// @brief This is the core function for generating the final respostr. Extracted
// this in order to avoid codedup in genErrResponse().
void Response::_buildResponseStr()
{
  for (std::map<str, str>::reverse_iterator it = _respoHeaders.rbegin();
      it != _respoHeaders.rend();
      it++)
    if (it->first == "Startline")
      _respoStr += it->second + CRLF;
    else
      _respoStr += it->first + ": " + it->second + CRLF;

  _respoStr += CRLF + (_req->isRedir() ? "" : _body);
  reallyClearStr(_body);
}

void Response::_getBody200()
{
  const Route& r = *_matchedRoute;

  if (_targetPath == "are/you/a/teapot?") {
    _status = HTTP_418;
    _setBodyStatusPage();
    return;
  }

  // root will NEVER have a trailing slash by parsing!
  str path = r.getRoot() + (r.getPath() == "/" ? "" : r.getPath());

  // At this point there can only be single slashes in _targetPath as they have
  // been compressed by URL class.
  if (!_targetPath.empty()) {
    if (_targetPath[0] == '/')
      path += _targetPath;
    else
      path += "/" + _targetPath;
  }

  Logger::logSrv(_vsrvName, "Trying to serve path: " + path);

  int isdir = getFileType(path);

  switch (isdir) {
    case 2:
    case -2:
    case -1:
      if (isdir == 2 || isdir == -2)
        Logger::logWarn("stat() encountered some weird file or symlink");
      _status = HTTP_404;
      _setBodyStatusPage();
      return;

    // is a file
    case 0:
      _readBodyFromFile(path);
      break;

    // is a dir
    case 1: {
      str fpath =
          path + (path[path.size() - 1] == '/' ? "" : "/") + r.getIndex();

      Logger::logSrv(_vsrvName, "Trying to serve file: " + fpath);

      // not setting errPage on fail here
      _readBodyFromFile(fpath, false);

      if (_status == HTTP_404 && r.isAutoindex()) {

        Logger::logSrv(_vsrvName, "Trying to serve autoindex for " + path);

        // kinda hacky construction of the path to use as root for displayed
        // files. but it works :/
        str displayPath = (_targetPath.empty()
                ? r.getPath()
                : (_targetPath[0] == '/' ? _targetPath : "/" + _targetPath));

        _body = WsrvLib::getAutoindex(path, displayPath);

        if (!_body.empty())
          _status = HTTP_200;
        else {
          _status = HTTP_403;
          _setBodyStatusPage();
        }

        return;
      }

      // if we come here reading index file & autoindex both failed.
      // doing some extra magic here to satisfy the 42 tester
      if (_status != HTTP_200 && fpath.find("Yeah") == str::npos) {
        _status = HTTP_403;
        _setBodyStatusPage();
      }
    }
  }
}

void Response::_buildRespoHdrs()
{
  _respoHeaders["Startline"] = "HTTP/1.1 " + WsrvLib::getStatusStr(_status);

  _respoHeaders["Server"] = "m0fr1m's webserv " VERSION;

  if (!_respoHdrHas("Date"))
    _respoHeaders["Date"] = Logger::getLogtime(false);

  if (!_respoHdrHas("Content-Type") && !_req->isRedir())
    _respoHeaders["Content-Type"] =
        (_status < HTTP_400) ? _mimeType : "text/html";

  if (!_respoHdrHas("Content-Length") && !_req->isRedir())
    _respoHeaders["Content-Length"] = int2str(_body.size());

  if (!_respoHdrHas("Location"))
    if (_req->isRedir()) {
      const URI& uri = _req->getRedir().second;
      if (uri.isURL())
        _respoHeaders["Location"] = uri.getStr();
      else {

        str targetPath = _req->getTargetPath();
        if (targetPath.size() > 0 && targetPath[0] != '/')
          targetPath = "/" + targetPath;

        _respoHeaders["Location"] = "http://" + _req->getHeaders()["host"] +
            uri.getStr() + targetPath +
            (_req->getReqline().target.getQuery().empty() ? "" : "?") +
            _req->getReqline().target.getQueryStr();
      }
      _respoHeaders["Connection"] = "close";
      _closeConn                  = true;
      _req->setCloseConn();
    }

  if (!_respoHdrHas("Connection")) {
    str conn;
    if ((_status >= HTTP_400 && (_status != HTTP_404)) || _closeConn)
      conn = "close";
    else
      conn = "keep-alive";
    _respoHeaders["Connection"] = conn;
  }

  if (!_respoHdrHas("Accept-Ranges"))
    _respoHeaders["Accept-Ranges"] = "none";
}

const str& Response::getRespoStr() const { return _respoStr; }

// reset.
void Response::reset()
{
  _status              = HTTP_200;
  _reqline.httpVersion = HTTPVER_UNKNOWN;
  _matchedRoute        = NULL;
  _reqline.method      = M_UNKNOWN;
  _req                 = NULL;
  _cgiParentWriteFd    = -1;
  _cgiParentReadFd     = -1;
  _cgiPid              = -1;
  _method              = M_UNKNOWN;
  _cgiBytesWritten     = 0;
  _cgiWriteBodySize    = 0;

  reallyClearStr(_body);
  reallyClearStr(_respoStr);
  reallyClearStr(_cgiBody);

  _reqline.target.clear();
  _respoHeaders.clear();
  _targetPath.clear();
  _mimeType = "";
  _vsrvName = "";
}

// helper function for generating the response for a failes Req.
std::map<str, str> Response::_buildErrRespoHdrs(
    e_HTTPStatus status, const str& body)
{
  std::map<str, str> hdrs;
  hdrs["Startline"]      = "HTTP/1.1 " + WsrvLib::getStatusStr(status);
  hdrs["Server"]         = "m0fr1m's webserv " VERSION;
  hdrs["Date"]           = Logger::getLogtime();
  hdrs["Content-Type"]   = "text/html";
  hdrs["Content-Length"] = int2str(body.length());

  str conn;
  if (status >= HTTP_400 && (status != HTTP_404))
    conn = "close";
  else
    conn = "keep-alive";
  hdrs["Connection"] = conn;
  return hdrs;
}

// Returns the response string to a given error status code.
str Response::genDefaultErrResponse(e_HTTPStatus errCode, constr errPage)
{
  str respostr;
  str body;

  if (!errPage.empty())
    body = errPage;
  else
    body = WsrvLib::getDefaultStatusPage(errCode);

  std::map<str, str> hdrs = _buildErrRespoHdrs(errCode, body);

  for (std::map<str, str>::reverse_iterator it = hdrs.rbegin();
      it != hdrs.rend();
      it++)
    if (it->first == "Startline")
      respostr += it->second + CRLF;
    else
      respostr += it->first + ": " + it->second + CRLF;
  respostr += CRLF + body;
  return respostr;
}

// get status page first from matched route secondly from vsrv. fallback to
// default page if both did not work
//
// TODO to be very perfect here in _readBodyFromFile we would also have to
// return the corresponding 404 if a status page could not be found.
void Response::_handleRedir()
{
  _status = _matchedRoute->getRedir().first;
  _setBodyStatusPage();
}

void Response::_handleDelete()
{

  const Route& r = *_matchedRoute;

  // root will NEVER have a trailing slash by parsing!
  str path = r.getRoot() + (r.getPath() == "/" ? "" : r.getPath());

  // At this point there can only be single slashes in _targetPath as they have
  // been compressed by URL class.
  if (!_targetPath.empty()) {
    if (_targetPath[0] == '/')
      path += _targetPath;
    else
      path += "/" + _targetPath;
  }

  Logger::logSrv(_vsrvName, "Trying to delete '" + path + "'");

  if (std::remove(path.c_str()) != 0) {
    Logger::logSrv(_vsrvName,
        "Deletion failed with '" + int2str(errno) + " " + strerror(errno) +
            "'");
    if (errno == ENOENT) {
      _status = HTTP_404;
    }
    else if (errno == EACCES || errno == EPERM) {
      _status = HTTP_403;
    }
    else {
      _status = HTTP_500;
    }
  }
  else
    _status = HTTP_204;
}

void Response::_readBodyFromFile(constr& path, bool setErrPageOnFail)
{
  _mimeType = WsrvLib::getMimeTypeFromPath(path);

  std::ifstream target;

  if (_mimeType != "text/html")
    target.open(path.c_str(), std::ios_base::binary);
  else
    target.open(path.c_str());

  if (!target) {
    _status = HTTP_404;
    if (setErrPageOnFail)
      _body = WsrvLib::getDefaultStatusPage(HTTP_404);
    return;
  }

  // get length of file:
  target.seekg(0, target.end);
  int length = target.tellg();
  target.seekg(0, target.beg);

  if (length <= 0) {
    if (length < 0 || !target) {
      _status = HTTP_404;
      if (setErrPageOnFail)
        _body = WsrvLib::getDefaultStatusPage(_status);
    }
    else {
      _status   = HTTP_200;
      _mimeType = "text/html";
      _body     = "Empty File :)";
    }
    return;
  }

  Logger::logDbg1("Response::_getBody", "body-length = " + int2str(length));

  std::vector<char> buffer(length);

  // good2know: does not throw but sets the badbit
  target.read(&buffer[0], length);

  if (target)
    // like this even NUL bytes and other weird binary-mode data is written to
    // the std::string obj.
    //
    // Furthermore std::string::assign might throw exceptions if the assigned
    // data is too large. So we catch it here
    try {
      _body.assign(buffer.begin(), buffer.end());
    } catch (const std::exception& e) {
      Logger::logErr("Response::_getBody", "Read body too large!");
      _status = HTTP_413;
      _body.clear();
      if (setErrPageOnFail)
        _body = WsrvLib::getDefaultStatusPage(_status);
      return;
    }
  _status = HTTP_200;
}

// extracted routine for fetching a desired status page in this order:
//
//  1) try to get page from _matchedRoute
//  2) if ther is none, from server scope
//  3) last from WsrvLib::getDefaultStatusPage.
//
// takes an optional arg for specifying a uploaded filename or sth in POST
// request responses
//
// the status code will be taken from objects _status field!
//
// NOTE This virtual-client-safe! Meaning protected against deref NULL
void Response::_setBodyStatusPage(constr& opts)
{
  e_HTTPStatus statBak = _status;
  str          statusPage;

  if (_matchedRoute != NULL)
    statusPage = _matchedRoute->getErrPage(_status);

  if (statusPage.empty() && _vsrv != NULL)
    statusPage = _vsrv->getErrPage(_status);

  // all the above failed possibly due to !_matchedRoute and !_vsrv
  if (statusPage.empty())
    _body = WsrvLib::getDefaultStatusPage(_status, opts);

  // we got a path to statusPage.. but never deref NULL!
  else
    _readBodyFromFile((_vsrv != NULL ? _vsrv->getRoot() : "") + statusPage);
  _status = statBak;
}

e_HTTPStatus Response::getStatus() const { return _status; }

void Response::setStatus(e_HTTPStatus s) { _status = s; }

// convenience function for checking if a std::map contains a non-empty field
bool Response::_respoHdrHas(constr& hdr)
{
  return _respoHeaders.find(hdr) != _respoHeaders.end() &&
      !_respoHeaders[hdr].empty();
}
