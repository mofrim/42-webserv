/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Route.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 16:42:51 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/14 22:14:59 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Route.hpp"

#include <stdexcept>

// --------------------------------=[ OCF ]=-------------------------------- //

Route::Route()
{
  _path        = "/";
  _root        = "";
  _autoindex   = false;
  _index       = "index.html";
  _maxBodySize = 0;
  _methods.insert(M_GET);
  _redir  = std::make_pair(HTTP_0, "");
  _upload = "";
}

// FIXME check if really all is copied!
Route::Route(const Route& o)
{
  if (this != &o) {
    _path        = o._path;
    _root        = o._root;
    _autoindex   = o._autoindex;
    _index       = o._index;
    _maxBodySize = o._maxBodySize;
    _errPages    = o._errPages;
    _methods     = o._methods;
    _redir       = o._redir;
    _cgi         = o._cgi;
    _upload      = o._upload;
  }
}

Route& Route::operator=(const Route& o)
{
  if (this != &o) {
    _path        = o._path;
    _root        = o._root;
    _autoindex   = o._autoindex;
    _index       = o._index;
    _maxBodySize = o._maxBodySize;
    _errPages    = o._errPages;
    _methods     = o._methods;
    _redir       = o._redir;
    _cgi         = o._cgi;
    _upload      = o._upload;
  }
  return (*this);
}

Route::~Route() {}

// ------------------------------=[ Methods ]=------------------------------ //

void Route::setPath(str p) { _path = p; }

str Route::getPath() const { return _path; }

void Route::setAutoindex(bool a) { _autoindex = a; }

bool Route::getAutoindex() const { return _autoindex; }

// we only set the default file if it does not contain any slashes which might
// lead to root folder escaping
//
// FIXME ... normally this should be taken care of elsewhere
void Route::setIndex(const str& s)
{
  if (s.find('/') == str::npos)
    _index = s;
}

str Route::getIndex() const { return _index; }

// Sanitizing the input string a little bit, as we don't want any trailing
// slashes as the path URL we append will start with a slash.
// Things like '///moep' will still be valid roots until a request is made.
void Route::setRoot(str root)
{
  if (root.empty())
    return;
  if (root[root.size() - 1] != '/')
    _root = root;
  else {
    Logger::logWarn(
        "Route::setRoot", "removing trailing slash from root '" + root + "'");
    str withoutTrailingSlash = root.substr(0, root.find('/'));
    if (!withoutTrailingSlash.empty())
      _root = withoutTrailingSlash;
  }
}
str Route::getRoot() const { return _root; }

// QUESTION bullet-proof?
void Route::reset()
{
  _path        = "/";
  _autoindex   = false;
  _index       = "index.html";
  _maxBodySize = 0;
  _redir       = std::make_pair(HTTP_0, "");

  _root.clear();
  _upload.clear();
  _methods.clear();
  _methods.insert(M_GET);
  _errPages.clear();
  _cgi.clear();
}

void Route::addErrPage(e_HTTPStatus s, const str& path) { _errPages[s] = path; }

// keep this for documenting how annoying using std::map::at() in C++98 is! It
// might throw out_of_range exception! So, better make every maps at least
// non-const ref in order to be able to use [] operator.
str Route::getErrPage(e_HTTPStatus s) const
{
  str ret;
  try {
    ret = _errPages.at(s);
  } catch (const std::out_of_range& e) {
    return "";
  }
  return ret;
}

std::map<e_HTTPStatus, str> Route::getErrPages() const { return _errPages; }

void Route::setMaxBodySize(u32 mbs) { _maxBodySize = mbs; }

u32 Route::getMaxBodySize() const { return _maxBodySize; }

const std::set<e_Method>& Route::getMethods() const { return _methods; }

bool Route::addMethod(e_Method m) { return _methods.insert(m).second; }

void Route::clearMethods() { _methods.clear(); }

const std::pair<e_HTTPStatus, str>& Route::getRedir() const { return _redir; }

void Route::setRedir(e_HTTPStatus s, const str& url)
{
  _redir = std::make_pair(s, url);
}

const std::map<str, str>& Route::getCgi() const { return _cgi; }

void Route::addCgi(constr& ext, constr& exec) { _cgi[ext] = exec; }

void Route::setUpload(str p) { _upload = p; }

str Route::getUpload() const { return _upload; }
