/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 11:26:23 by fmaurer           #+#    #+#             */
/*   Updated: 2026/06/13 12:18:19 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WsrvLib.hpp"

#include <netinet/in.h>
#include <set>
#include <sstream>
#include <vector>

str bool2str(bool n);
str char2str(char c);

str inAddrToStr(const struct in_addr& addr);
str getAddrPortStr4(const struct sockaddr_in& addr);

int setFdNonBlocking(int fd);
int setFdCloexec(int fd);

// TODO put them in WsrvLib
e_Method str2meth(constr& m);
str      meth2str(e_Method m);

int  getFileType(constr& path);
str  strip(str str);
void toLowerInPlace(str& s);
str  toLower(str s);
u16  str2u16(constr& s);

str data2hexStr(const char *s, size_t len);
str printDataTrunc(const char *s, size_t len, size_t trunc);

bool   isValidFnameChar(char c);
bool   isValidFname(const str& s);
str    getErrnoStr();
size_t findCRLF(const char *dat, size_t len);
bool   parseU32HexStr(const str& hexStr, uint32_t& out);
void   reallyClearStr(str& s);

std::set<str> listDirFiles(constr& directoryPath, bool dirSlash = true);

// -----------------------------=[ splitters ]=----------------------------- //

std::vector<str> splitString(
    constr& sstr, constr& delim, bool keepEmpty = false);

std::vector<str> splitStrWhite(constr& sstr, bool keepEmpty = false);

std::vector<str> splitMultiStr(
    const str& sstr, constr& delims, bool keepEmpty = false);

// -----------------------------=[ Templates ]=----------------------------- //

// general number to string conversion func
template <typename T>
str int2str(T n)
{
  std::ostringstream oss;
  oss << n;
  return (oss.str());
}

// a convenience printing function for sets mostly used for logging
template <typename T>
std::ostream& operator<<(std::ostream& os, std::set<T> lst)
{
  if (lst.size() == 0) {
    os << "";
    return os;
  }
  os << "{";
  for (std::set<u16>::iterator it = lst.begin(); it != lst.end();)
    os << *it << (++it != lst.end() ? ", " : "}");
  return os;
}

// convenience function for outputting sets in logging
template <typename T>
str getSetAsStr(const std::set<T>& s)
{
  str ret("");
  if (s.size() == 0)
    return ret;
  ret += "{";
  for (typename std::set<T>::iterator it = s.begin(); it != s.end();)
    ret += int2str(*it) + (++it != s.end() ? ", " : "}");
  return ret;
}

// convenience function for outputting sets in logging
template <typename T>
str getStrSetAsStr(const std::set<T>& s)
{
  str ret("");
  if (s.size() == 0)
    return ret;
  ret += "{";
  for (typename std::set<T>::iterator it = s.begin(); it != s.end();)
    ret += *it + (++it != s.end() ? ", " : "}");
  return ret;
}

// getSetAsStr in a version that supports printing sets of pairs like i use in
// VServer::_activeInterfaces and specifially there in t_vservInterface.portFd
template <typename T1, typename T2>
str getSetAsStr(const std::set< std::pair<T1, T2> >& s)
{
  str ret("");
  if (s.size() == 0)
    return ret;
  ret += "{";
  for (typename std::set< std::pair<T1, T2> >::iterator it = s.begin();
      it != s.end();)
    ret += "[" + int2str(it->first) + "/fd=" + int2str(it->second) + "]" +
        (++it != s.end() ? ", " : "}");
  return ret;
}

// @brief Erase an element from the same container param it is currently
//        iterating over.
//
// CAUTION: only use in while loops because otherwise itertor will be
// overincremented!!!
//
// @param conti Container `it` is an iterator to
// @param it Container::iterator
//
// well in C++11 std::map::erase and std::set::erase return an iterator to the
// next elem after erasure. in C++98 they don't. that is a problem because like
// that it is kind of difficult to safely iterate over a container while
// possibly deleting elems from it.
//
// Example of safe usage:
//
//    std::vector<int>::it = vec.begin();
//    while (it != vec.end())
//    {
//      if (condition) {
//        it = eraseIt(vec, it);
//        continue;
//      }
//      ...
//      ++it;
//    }
template <typename T>
typename T::iterator eraseIt(T& conti, typename T::iterator it)
{
  typename T::iterator itBak = it;
  ++it;
  conti.erase(itBak);
  return it;
}
