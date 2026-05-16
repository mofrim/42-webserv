/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   URI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/06 22:32:39 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/16 12:17:02 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "URI.hpp"
#include "utils.hpp"

#include <iostream>
#include <sstream>
#include <vector>

// --------------------------------=[ OCF ]=-------------------------------- //

URI::URI(): _bad(false), _empty(true) {}

URI::URI(const URI& o)
{
  if (this != &o)
    *this = o;
}

URI& URI::operator=(const URI& o)
{
  if (this != &o) {
    _scheme   = o._scheme;
    _auth     = o._auth;
    _path     = o._path;
    _query    = o._query;
    _fragment = o._fragment;
    _bad      = o._bad;
    _empty    = o._empty;
  }
  return (*this);
}

URI::~URI() {}

// ------------------------------=[ END OCF ]=------------------------------ //

URI::URI(const str& u)
{
  this->parseURL(u);
  if (_bad) {
    std::cout << "parseURL failed!" << std::endl;
    this->parsePath(u);
  }
}

// ---------------------=[ URL parsing static helpers ]=--------------------- //

// Helper function to decode percent-encoded characters
static str percentDecode(const str& str)
{
  std::ostringstream decoded;
  for (size_t i = 0; i < str.size(); ++i) {
    if (str[i] == '%' && i + 2 < str.size()) {
      int hex1 = std::tolower(str[i + 1]);
      int hex2 = std::tolower(str[i + 2]);

      if (std::isxdigit(hex1) && std::isxdigit(hex2)) {
        int value =
            (std::isdigit(hex1) ? (hex1 - '0') : (hex1 - 'a' + 10)) * 16 +
            (std::isdigit(hex2) ? (hex2 - '0') : (hex2 - 'a' + 10));
        decoded << static_cast<char>(value);
        i += 2;
      }
      else
        decoded << str[i];
    }
    else
      decoded << str[i];
  }
  return decoded.str();
}

// helper function to check for bad characters (null bytes, control chars, etc.)
static bool hasBadChars(const str& s)
{
  for (str::const_iterator it = s.begin(); it != s.end(); it++)
    if (*it == '\0' || std::iscntrl(static_cast<unsigned char>(*it)))
      return true;
  return false;
}

// helper function to split query parameters into key-value pairs
static std::map<str, str> parseQueryParams(const str& query)
{
  std::map<str, str> params;
  if (query.empty() || query[0] != '?') {
    return params;
  }

  str                query_str = query.substr(1);
  std::istringstream iss(query_str);
  str                param;

  while (std::getline(iss, param, '&')) {
    size_t equal_pos = param.find('=');
    str    key, value;

    if (equal_pos != str::npos) {
      key   = param.substr(0, equal_pos);
      value = param.substr(equal_pos + 1);
    }
    else {
      key   = param;
      value = "";
    }

    key   = percentDecode(key);
    value = percentDecode(value);

    params.insert(std::make_pair(key, value));
  }

  return params;
}

// Helper function to reconstruct the query string from parsed parameters
static str rebuildQueryStr(const std::map<str, str>& params)
{
  if (params.empty()) {
    return "";
  }

  std::ostringstream query;
  query << "?";

  for (std::map<str, str>::const_iterator it = params.begin();
      it != params.end();
      ++it)
  {
    if (it != params.begin())
      query << "&";
    query << it->first;
    if (!it->second.empty())
      query << "=" << it->second;
  }

  return query.str();
}

// ----------------------=[ The mai URL parsing func ]=---------------------- //

// the main target-path/url parsing function
//
// i only support origin-form target URLs, so anything not starting with "/" is
// a bad URL.
//
// Returns either an empty string on failure (also sets bad-bool in this case)
// or the fully sanitized URL string. As side-effects sets all the corresponding
// fields of the object: _path, _query and _fragment. As we are a server there
// is no point in supporting fragments but it does not hurt to filter them out.
str URI::parsePath(const str& target)
{
  this->clear();

  if (target.empty())
    return "/";

  if (target[0] != '/') {
    _bad = true;
    return "";
  }

  size_t queryPos    = target.find('?');
  size_t fragmentPos = target.find('#');

  str query;

  if (fragmentPos != str::npos) {
    _fragment = target.substr(fragmentPos);
    if (queryPos != str::npos && queryPos < fragmentPos) {
      _path = target.substr(0, queryPos);
      query = target.substr(queryPos, fragmentPos - queryPos);
    }
    else
      _path = target.substr(0, fragmentPos);
  }
  else if (queryPos != str::npos) {
    _path = target.substr(0, queryPos);
    query = target.substr(queryPos);
  }
  else {
    _path = target;
  }

  // no more plain slashes allowed in query or fragment strings
  if (_fragment.find('/') != str::npos || query.find('/') != str::npos) {
    _bad = true;
    return "";
  }

  str decodedPath = percentDecode(_path);

  if (hasBadChars(decodedPath)) {
    _bad = true;
    return "";
  }

  std::vector<str> segments = splitString(decodedPath, "/");

  std::vector<str> resolvedSegments;
  for (std::vector<str>::iterator it = segments.begin(); it != segments.end();
      ++it)
  {
    if (*it == ".")
      continue;
    else if (*it == "..") {
      if (!resolvedSegments.empty())
        resolvedSegments.pop_back();
    }
    else
      resolvedSegments.push_back(*it);
  }

  // building sanitized_path here filtering out possible double slashes
  str sanitizedPath;
  for (std::vector<str>::iterator it = resolvedSegments.begin();
      it != resolvedSegments.end();
      ++it)
  {
    if (!sanitizedPath.empty() &&
        sanitizedPath[sanitizedPath.size() - 1] != '/')
      sanitizedPath += "/" + *it;
    else
      sanitizedPath += *it;
  }

  // ensuring origin-form here again
  if (sanitizedPath.empty())
    sanitizedPath = "/";

  if (sanitizedPath[0] != '/')
    sanitizedPath = "/" + sanitizedPath;

  char lastTargetChar = target[target.size() - 1];
  if ((lastTargetChar == '/' || lastTargetChar == '.') && sanitizedPath != "/")
    sanitizedPath = sanitizedPath + "/";

  _path = sanitizedPath;

  str sanitizedQueryStr = query;
  if (!query.empty()) {
    _query = parseQueryParams(query);
    if (_query.empty())
      sanitizedQueryStr = "";
    else
      sanitizedQueryStr = rebuildQueryStr(_query);
  }

  str saniURL = sanitizedPath + sanitizedQueryStr + _fragment;

  _bad   = false;
  _empty = false;

  return saniURL;
}

// ------------------------=[ The usual one-liners ]=------------------------ //

bool                URI::bad() const { return _bad; }
bool                URI::empty() const { return _empty; }
str                 URI::getPath() const { return _path; }
std::map<str, str>& URI::getQuery() { return _query; }

// if _scheme is empty we are merely a path not a full URL
bool URI::isURL() const { return !_scheme.empty(); }

str URI::getStr() const
{
  str ret;
  if (!_scheme.empty() && !_auth.empty())
    ret += _scheme + "://" + _auth;
  ret += _path;
  if (!_query.empty())
    ret += "?" + this->getQueryStr();
  if (!_fragment.empty())
    ret += "#" + _fragment;
  return ret;
}

void URI::clear()
{
  _scheme.clear();
  _auth.clear();
  _path.clear();
  _query.clear();
  _fragment.clear();
  _empty = true;
  _bad   = false;
}

// return the query key-value-map as a Comma Separated String
str URI::getQueryCSStr() const
{
  str ret;

  for (std::map<str, str>::const_iterator it = _query.begin();
      it != _query.end();)
  {
    ret += it->first + "=" + it->second + (++it != _query.end() ? ", " : "");
  }
  return ret;
}

// return query string in URL compatible format
str URI::getQueryStr() const
{
  str ret;

  for (std::map<str, str>::const_iterator it = _query.begin();
      it != _query.end();)
  {
    ret += it->first + "=" + it->second + (++it != _query.end() ? "&" : "");
  }

  return ret;
}

// ----------------------=[ limited URL parsing func ]=---------------------- //

static bool isValidAuthChar(char c, bool portSep)
{
  if (portSep)
    return isdigit(c);
  return (isalpha(c) || isdigit(c) || c == '.' || c == '-' || c == ':');
}

// QND chekc if string is valid u16 for port
bool isU16Str(constr& str)
{
  if (str.empty())
    return false;

  size_t i = 0;

  u32 value = 0;
  while (i < str.length() && isdigit(str[i])) {
    u8 digit = str[i] - '0';
    if (value > (65535 - digit) / 10)
      return false;
    value = value * 10 + digit;
    ++i;
  }

  if (i < str.length() && !isdigit(str[i]))
    return false;

  return value <= 65535;
}

// See:
// - https://datatracker.ietf.org/doc/html/rfc3986
// - https://datatracker.ietf.org/doc/html/rfc9110#name-identifiers-in-http
//
// NOTE This only a veeeeery reduced form of URL parsing which is not at all RFC
// compliant!
str URI::parseURL(constr& u)
{
  this->clear();

  if (u.length() < 7) {
    _bad = true;
    return "";
  }
  if (u.compare(0, 7, "http://") != 0 &&
      (u.length() >= 8 && u.compare(0, 8, "https://") != 0))
  {
    _bad = true;
    return "";
  }

  str scheme = u.substr(0, u.find("://"));

  str    auth    = u.substr(u.find("//") + 2);
  size_t hierEnd = auth.find("/");

  // re-use my path parsing func
  if (hierEnd != str::npos)
    this->parsePath(auth.substr(hierEnd));

  auth = auth.substr(0, hierEnd);

  if (auth.length() == 0 || auth.length() > URL_MAX_AUTH_LENGTH) {
    _bad = true;
    return "";
  }

  if (!isalnum(auth[0])) {
    _bad = true;
    return "";
  }

  bool portSep = false;

  // max port: 65535!
  // no trailing ':' allowed!
  for (size_t i = 0; i < auth.length() && !_bad; ++i) {
    if (!isValidAuthChar(auth[i], portSep))
      _bad = true;
    if (i + 1 < auth.length())
      if (!auth.compare(i, 2, "..") || !auth.compare(i, 2, ".-") ||
          !auth.compare(i, 2, "-.") || !auth.compare(i, 2, ":.") ||
          !auth.compare(i, 2, ".:") || !auth.compare(i, 2, "-:") ||
          !auth.compare(i, 2, ":-"))
        _bad = true;
    if (auth[i] == ':' && (portSep || i + 1 == auth.length()))
      _bad = true;
    else if (auth[i] == ':' && !portSep)
      portSep = true;
  }

  if (_bad)
    return "";

  if (portSep && !isU16Str(auth.substr(auth.rfind(":") + 1))) {
    _bad = true;
    return "";
  }

  if (auth[auth.length() - 1] == '.' || auth[auth.length() - 1] == '-') {
    _bad = true;
    return "";
  }

  _scheme = scheme;
  _auth   = auth;
  _empty  = false;
  return auth;
}
