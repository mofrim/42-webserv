/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 10:03:57 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/27 12:18:11 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <iomanip>
#include <iostream>
#include <sys/stat.h>

str char2str(char c)
{
  std::ostringstream oss;
  oss << c;
  return (oss.str());
}

str bool2str(bool n)
{
  if (n)
    return std::string("true");
  else
    return std::string("false");
}

// our own inet_ntoa function as by subject.pdf we figured that we shouldn't use
// the std function.
//
// converts ip-addr from network byte order (big endian) to string. non-portable
// little-endian version
str inAddrToStr(const struct in_addr& addr)
{
  str ret("");
  ret.append(int2str(addr.s_addr & 255) + "." +
      int2str((addr.s_addr >> 8) & 255) + "." +
      int2str((addr.s_addr >> 16) & 255) + "." +
      int2str((addr.s_addr >> 24) & 255));
  return (ret);
}

int setFdNonBlocking(int fd)
{
  int flags = fcntl(fd, F_GETFL);
  return (fcntl(fd, F_SETFL, flags | O_NONBLOCK));
}

// sets the FD_CLOEXEC flag on a fd. returns 0 on success, -1 otherwise.
int setFdCloexec(int fd)
{
  int flags = fcntl(fd, F_GETFD);
  if (flags == -1)
    return -1;
  flags |= FD_CLOEXEC;
  if (fcntl(fd, F_SETFD, flags) == -1)
    return -1;
  return 0;
}

// returns a string of format "localhost:1234" or "42.42.42.1:23" for valid IPv4
// adresses.
str getAddrPortStr4(const struct sockaddr_in& addr)
{
  str ret("");
  ret += inAddrToStr(addr.sin_addr);
  ret += ":" + int2str(ntohs(addr.sin_port));
  return (ret);
}

e_Method str2meth(const str& m)
{
  if (m == "GET")
    return M_GET;
  if (m == "POST")
    return M_POST;
  if (m == "DELETE")
    return M_DELETE;
  return M_UNKNOWN;
}

str meth2str(e_Method m)
{
  if (m == M_GET)
    return "GET";
  if (m == M_POST)
    return "POST";
  if (m == M_DELETE)
    return "DELETE";
  return "UNKNOWN";
}

// check if given path is
//
//  - inexistent -> return -1
//  - regular file -> return 0
//  - dir -> return 1
//  - link -> return 2 (technically cannot happen with stat)
//  - sth else -> return -2
int getFileType(const str& path)
{
  struct stat sb;

  if (stat(path.c_str(), &sb) == -1)
    return -1;

  switch (sb.st_mode & S_IFMT) {
    case S_IFREG:
      return 0;
    case S_IFDIR:
      return 1;
    case S_IFLNK:
      return 2;
    default:
      return -2;
  }
}

// @brief Split string by another delim string.
// @param keepEmpty set this to true for keeping empty lines after split
// @return  an std::vector filled with the split strings. if there is no delim
//          in sstr or any param is empty, returs vector with sstr as only elem
std::vector<str> splitString(const str& sstr, const str& delim, bool keepEmpty)
{
  std::vector<str> ret;
  size_t           i = 0;
  size_t           k = 0;

  if (sstr.empty() || delim.empty()) {
    ret.push_back(sstr);
    return ret;
  }

  while (i < sstr.size() && k != str::npos) {
    k = sstr.find(delim, i);
    str sub;
    if (k == str::npos)
      sub = sstr.substr(i, str::npos);
    else
      sub = sstr.substr(i, k - i);
    if (!sub.empty() || keepEmpty)
      ret.push_back(sub);
    i = k + delim.size();
  }
  return ret;
}

// @brief Split string by whitespace chars
// @param keepEmpty set this to true for keeping empty lines after split
// @return  an std::vector filled with the split strings.
std::vector<str> splitStrWhite(const str& sstr, bool keepEmpty)
{
  std::vector<str> ret;
  size_t           i = 0;
  size_t           k = 0;

  const str ws(" \t\v\f\n");

  if (sstr.empty())
    return ret;

  i = sstr.find_first_not_of(ws);
  if (i == str::npos)
    return ret;

  while (i < sstr.size() && k != str::npos) {
    k = sstr.find_first_of(ws, i);
    str sub;
    if (k == str::npos)
      sub = sstr.substr(i, str::npos);
    else
      sub = sstr.substr(i, k - i);
    if (!sub.empty() || keepEmpty)
      ret.push_back(sub);
    while (k < sstr.size() && isspace(sstr[k]))
      ++k;
    i = k;
  }
  return ret;
}

std::vector<str> splitMultiStr(const str& sstr, constr& delims, bool keepEmpty)
{
  std::vector<str> ret;
  size_t           i = 0;
  size_t           k = 0;

  if (sstr.empty())
    return ret;

  if (delims.empty()) {
    ret.push_back(sstr);
    return ret;
  }

  i = sstr.find_first_not_of(delims);
  if (i == str::npos)
    return ret;

  while (i < sstr.size() && k != str::npos) {
    k = sstr.find_first_of(delims, i);
    str sub;
    if (k == str::npos)
      sub = sstr.substr(i, str::npos);
    else
      sub = sstr.substr(i, k - i);
    if (!sub.empty() || keepEmpty)
      ret.push_back(sub);
    while (k < sstr.size() && delims.find(sstr[k]) != str::npos)
      ++k;
    i = k;
  }
  return ret;
}

// @brief remove leading and trailing whitespace chars from string
str strip(str s)
{
  if (s.length() == 0)
    return s;
  std::string::iterator it0 = s.begin();
  while (isspace(*it0) && it0 != s.end())
    it0++;
  std::string::iterator it1 = s.end();

  // annoyingly have to take a step backwards because str.end() is one past the
  // end and migth actually be no ws char.
  --it1;

  while (isspace(*it1) && it1 != s.begin())
    --it1;

  // yep. and again, the range-ctor is excluding the end. this is not what we
  // want, so we need to increment.
  //
  // NOTE: this can fail with std::length_error exception when the string, we
  // are trying to create here is longer than std::string::max_size(). this is
  // why we wrap it.
  str stripped;
  try {
    stripped = std::string(it0, ++it1);
  } catch (const std::length_error& e) {
    std::cout << "strip: std::length_error: " << e.what() << std::endl;
  }
  return stripped;
}

// an in-place tolower function because internally we will work case-insensitive
// with header fields
void toLowerInPlace(str& s)
{
  for (size_t i = 0; i < s.size(); i++)
    s[i] = std::tolower(s[i]);
}

// a not-in-place tolower
str toLower(str s)
{
  str low;
  low.resize(s.size());
  for (size_t i = 0; i < s.size(); i++)
    low[i] = std::tolower(s[i]);
  return low;
}

// veeery simple conversion to u16 without any overflow checking.
u16 str2u16(const str& s) { return static_cast<u16>(atoi(s.c_str())); }

// print a given std::string's raw bytes as double digit hex numbers prefixed by
// `\x` like nginx does with non-sense input. printable chars are just printed.
str data2hexStr(const char *s, size_t len)
{
  std::ostringstream oss;
  for (size_t i = 0; i < len; ++i) {
    if (isprint(s[i]))
      oss << s[i];
    else
      oss << "\\x" << std::hex << std::setw(2) << std::uppercase
          << std::setfill('0')
          << static_cast<int>(static_cast<unsigned char>(s[i]));
  }
  return oss.str();
}

bool isValidFnameChar(char c)
{
  return isalnum(c) || c == '.' || c == '-' || c == '_';
}

// arbitraily deciding the filenames should not begin with '-'. idk, but that
// seems fishy to me.
bool isValidFname(const str& s)
{
  if (s.length() == 0)
    return false;

  str::const_iterator it = s.begin();
  if (*it == '-')
    return false;

  for (; it != s.end(); ++it)
    if (!isValidFnameChar(*it))
      return false;

  return true;
}

// ls
//
// will return an empty set if sth went wrong. the set of files in the dir
// otherwise. if dirSlash param is true add a '/' to every dir.
std::set<str> listDirFiles(constr& directoryPath, bool dirSlash)
{
  DIR           *dir;
  struct dirent *entry;
  std::set<str>  files;
  str            fname;

  if ((dir = opendir(directoryPath.c_str())) != NULL) {

    while ((entry = readdir(dir)) != NULL) {
      fname = entry->d_name;
      if (fname != ".")
        files.insert(
            fname + ((entry->d_type == DT_DIR && dirSlash) ? "/" : ""));
    }
    closedir(dir);
  }

  return files;
}

// return a nicely formatted errno string
str getErrnoStr()
{
  return str("errno: \"#" + int2str(errno) + " " + strerror(errno) + "\"");
}
