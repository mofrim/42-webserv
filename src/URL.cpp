/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   URL.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/06 22:32:39 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/07 08:29:57 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "URL.hpp"

#include <sstream>
#include <vector>

// --------------------------------=[ OCF ]=-------------------------------- //

URL::URL(): _bad(false) {}

URL::URL(const URL& o)
{
  if (this != &o) {
    _path     = o._path;
    _query    = o._query;
    _fragment = o._fragment;
    _bad      = o._bad;
  }
}

URL& URL::operator=(const URL& o)
{
  if (this != &o) {
    _path     = o._path;
    _query    = o._query;
    _fragment = o._fragment;
    _bad      = o._bad;
  }
  return (*this);
}

URL::~URL() {}

URL::URL(const str& u) { (void)u; }

// ----------------------------=[ URL Parsing ]=---------------------------- //

// Helper function to decode percent-encoded characters
str percentDecode(const str& str)
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

// Helper function to check for bad characters (null bytes, control chars, etc.)
bool containsBadCharacters(const str& s)
{
  for (std::string::const_iterator it = s.begin(); it != s.end(); it++)
    if (*it == '\0' || std::iscntrl(static_cast<unsigned char>(*it)))
      return true;
  return false;
}

// Helper function to split query parameters into key-value pairs
std::vector<std::pair<str, str> > parseQueryParameters(const str& query)
{
  std::vector<std::pair<str, str> > params;
  if (query.empty() || query[0] != '?') {
    return params;
  }

  str                query_str = query.substr(1); // Remove the leading '?'
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

    // Decode percent-encoded characters in key and value
    key   = percentDecode(key);
    value = percentDecode(value);

    params.push_back(std::make_pair(key, value));
  }

  return params;
}

// Helper function to reconstruct the query string from parsed parameters
str reconstructQueryString(const std::vector<std::pair<str, str> >& params)
{
  if (params.empty()) {
    return "";
  }

  std::ostringstream query;
  query << "?";

  for (size_t i = 0; i < params.size(); ++i) {
    if (i > 0) {
      query << "&";
    }
    query << params[i].first;
    if (!params[i].second.empty()) {
      query << "=" << params[i].second;
    }
  }

  return query.str();
}

str URL::sanitizeTargetURL(const str& target)
{
  if (target.empty()) {
    return "/";
  }

  size_t queryPos    = target.find('?');
  size_t fragmentPos = target.find('#');

  str path;
  str query;
  str fragment;

  if (fragmentPos != str::npos) {
    fragment = target.substr(fragmentPos);
    if (queryPos != str::npos && queryPos < fragmentPos) {
      path  = target.substr(0, queryPos);
      query = target.substr(queryPos, fragmentPos - queryPos);
    }
    else {
      path = target.substr(0, fragmentPos);
    }
  }
  else if (queryPos != str::npos) {
    path  = target.substr(0, queryPos);
    query = target.substr(queryPos);
    Logger::log_bug("query: " + query);
  }
  else {
    path = target;
  }

  str decoded_path = percentDecode(path);

  if (containsBadCharacters(decoded_path))
    throw std::invalid_argument("URL contains invalid characters");

  std::vector<str>   segments;
  std::istringstream iss(decoded_path);
  str                segment;

  while (std::getline(iss, segment, '/'))
    if (!segment.empty())
      segments.push_back(segment);

  std::vector<str> resolved_segments;
  for (std::vector<str>::iterator it = segments.begin(); it != segments.end();
      ++it)
  {
    if (*it == ".")
      resolved_segments.push_back("");
    else if (*it == "..") {
      if (!resolved_segments.empty())
        resolved_segments.pop_back();
    }
    else
      resolved_segments.push_back(*it);
  }

  // building sanitized_path here filtering out possible double slashes
  str sanitized_path;
  for (std::vector<str>::iterator it = resolved_segments.begin();
      it != resolved_segments.end();
      ++it)
  {
    if (!sanitized_path.empty() &&
        sanitized_path[sanitized_path.size() - 1] != '/')
      sanitized_path += "/" + *it;
    else
      sanitized_path += *it;
  }

  if (sanitized_path.empty())
    sanitized_path = "/";

  str sanitizedQuery = query;
  if (!query.empty()) {
    std::vector<std::pair<str, str> > params = parseQueryParameters(query);
    if (params.empty())
      sanitizedQuery = "";
    else
      sanitizedQuery = reconstructQueryString(params);
  }

  str sanitized = sanitized_path + sanitizedQuery + fragment;

  // Step 8: Ensure the path is absolute
  if (sanitized[0] != '/') {
    sanitized = "/" + sanitized;
  }

  return sanitized;
}
