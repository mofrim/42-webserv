/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 11:26:23 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/20 11:47:36 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "typesAndConstants.hpp"

#include <netinet/in.h>
#include <set>
#include <sstream>

str int2str(int n);
str inAddrToStr(const struct in_addr& addr);
str getAddrPortStr4(const struct sockaddr_in& addr);
int setFdNonBlocking(int fd);

// -----------------------------=[ Templates ]=----------------------------- //

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

// again a convenience function for outputting sets in logging
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
