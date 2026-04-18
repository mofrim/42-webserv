/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/26 10:03:57 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/18 15:38:25 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// TODO: add missing conversion functions here.

#include "utils.hpp"

#include <fcntl.h>
#include <sstream>

std::string int2str(int n)
{
  std::ostringstream oss;
  oss << n;
  return (oss.str());
}

// our own inet_ntoa function as by subject.pdf we figured that we shouldn't use
// the std function.
//
// converts ip-addr from network byte order (big endian) to string.
// TODO: make portable. is there some constant we can check for the used
// network-byte-order and host-byte-order?
std::string inAddrToStr(const struct in_addr& addr)
{
  std::string ret("");
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

// returns a string of format "localhost:1234" or "42.42.42.1:23" for valid IPv4
// adresses.
std::string getAddrPortStr4(const struct sockaddr_in& addr)
{
  std::string ret("");
  ret += inAddrToStr(addr.sin_addr);
  ret += ":" + int2str(ntohs(addr.sin_port));
  return (ret);
}

// TODO: implement inet_pton() for

// an easy output operator for a set of ports 8)
std::ostream& operator<<(std::ostream& os, std::set<u16> lst)
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
