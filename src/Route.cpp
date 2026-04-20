/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Route.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/09 16:42:51 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/20 20:35:29 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Route.hpp"

// --------------------------------=[ OCF ]=-------------------------------- //

Route::Route()
{
  _root         = "html";
  _autoindex    = false;
  _default_file = "index.html";
}

Route::Route(const Route& o)
{
  if (this != &o) {
    _root         = o._root;
    _autoindex    = o._autoindex;
    _default_file = o._default_file;
  }
}

Route& Route::operator=(const Route& o)
{
  if (this != &o) {
    _root         = o._root;
    _autoindex    = o._autoindex;
    _default_file = o._default_file;
  }
  return (*this);
}

Route::~Route()
{}

// ------------------------------=[ Methods ]=------------------------------ //
//
void Route::setPath(str p)
{
  _path = p;
}

str Route::getPath() const
{
  return _path;
}

void Route::setAutoindex(bool a)
{
  _autoindex = a;
}

bool Route::getAutoindex() const
{
  return _autoindex;
}

void Route::setDefaultFile(const str& s)
{
  _default_file = s;
}

str Route::getDefaultFile() const
{
  return _default_file;
}

void Route::setRoot(str root)
{
  _root = root;
}

str Route::getRoot() const
{
  return _root;
}
