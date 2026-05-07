/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Route.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 16:42:51 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/07 13:16:12 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Route.hpp"

// --------------------------------=[ OCF ]=-------------------------------- //

Route::Route()
{
  _path         = "/";
  _root         = "./html";
  _autoindex    = false;
  _default_file = "index.html";
}

Route::Route(const Route& o)
{
  if (this != &o) {
    _path         = o._path;
    _root         = o._root;
    _autoindex    = o._autoindex;
    _default_file = o._default_file;
  }
}

Route& Route::operator=(const Route& o)
{
  if (this != &o) {
    _path         = o._path;
    _root         = o._root;
    _autoindex    = o._autoindex;
    _default_file = o._default_file;
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
void Route::setDefaultFile(const str& s)
{
  if (s.find('/') == str::npos)
    _default_file = s;
}

str Route::getDefaultFile() const { return _default_file; }

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
    Logger::log_warn(
        "Route::setRoot", "removing trailing slash from root '" + root + "'");
    str withoutTrailingSlash = root.substr(0, root.find('/'));
    if (!withoutTrailingSlash.empty())
      _root = withoutTrailingSlash;
  }
}

str Route::getRoot() const { return _root; }
