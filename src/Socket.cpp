/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/28 07:26:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/20 13:28:08 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"
#include "Socket.hpp"
#include "utils.hpp"

#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <netdb.h>

// --------------------------------=[ OCF ]=-------------------------------- //

Socket::Socket()
{}

Socket::Socket(const Socket& other)
{
  (void)other;
}

Socket& Socket::operator=(const Socket& other)
{
  (void)other;
  return (*this);
}

Socket::~Socket()
{}

// ------------------------------=[ Methods ]=------------------------------ //

// AI_NUMERICSERV: forces interpretion of port number passed to getaddrinfo as
// numeric -> there is no service name lookup performed in /etc/services.
//
// AI_NUMERICHOST: would do the same for the hostname/addr
struct addrinfo *Socket::getAddrInfo(const str& addr, u16 port)
{
  struct addrinfo  hints;
  struct addrinfo *result;
  int              s;

  memset(&hints, 0, sizeof(struct addrinfo));

  hints.ai_canonname = NULL;
  hints.ai_addr      = NULL;
  hints.ai_next      = NULL;
  hints.ai_family    = AF_INET;
  hints.ai_socktype  = SOCK_STREAM;
  hints.ai_flags     = AI_PASSIVE | AI_NUMERICSERV | AI_CANONNAME;

  s = getaddrinfo(addr.c_str(), int2str(port).c_str(), &hints, &result);

  if (s != 0) {
    throw(AddrInfoException(
        "(" + addr + ", " + int2str(port) + ") " + gai_strerror(s)));
  }
  return result;
}

// FIXME: implement/use own version of inet_ntop() as it is not allowed by
// subject.
void Socket::printAddrlist(const str& addr, u16 port)
{
  struct addrinfo *ai;
  struct addrinfo *ap;

  try {
    ai = getAddrInfo(addr, port);
  } catch (const std::runtime_error& e) {
    throw;
  }

  std::string out("addrinfo for " + addr + ":" + int2str(port) + "\n");
  out += "  ---\n";

  ap = ai;
  while (ap != NULL) {
    switch (ap->ai_family) {
    case AF_UNSPEC:
      out += "  ai_family: AF_UNSPEC\n";
      break;
    case AF_INET:
      out += "  ai_family: AF_INET\n";
      break;
    case AF_INET6:
      out += "  ai_family: AF_INET6\n";
      break;
    default:
      out += "  ai_family: unknown\n";
    }

    switch (ap->ai_socktype) {
    case SOCK_STREAM:
      out += "  ai_socktype: SOCK_STREAM\n";
      break;
    case SOCK_DGRAM:
      out += "  ai_socktype: SOCK_DGRAM\n";
      break;
    default:
      out += "  ai_socktype: 0\n";
    }

    switch (ap->ai_protocol) {
    case IPPROTO_TCP:
      out += "  ai_protocol: IPPROTO_TCP\n";
      break;
    case IPPROTO_UDP:
      out += "  ai_protocol: IPPROTO_UDP\n";
      break;
    default:
      out += "  ai_protocol: 0\n";
    }

    out += "  ai_addrlen: " + int2str(ap->ai_addrlen) + "\n";

    char ip[INET6_ADDRSTRLEN];
    int  port = 0;
    if (ap->ai_family == AF_INET) {
      if (inet_ntop(ap->ai_family,
              &((struct sockaddr_in *)ap->ai_addr)->sin_addr,
              ip,
              INET6_ADDRSTRLEN) != NULL)
        out += "  ai_addr: " + std::string(ip) + ", ";
      else
        out += "  ai_addr: " + getErrStr() + ", ";

      port = ntohs(((struct sockaddr_in *)ap->ai_addr)->sin_port);
      out += "port: " + int2str(port) + "\n";
    }
    else {
      if (inet_ntop(ap->ai_family,
              &((struct sockaddr_in6 *)ap->ai_addr)->sin6_addr,
              ip,
              INET6_ADDRSTRLEN) != NULL)
        out += "  ai_addr: " + str(ip) + ", ";
      else
        out += "  ai_addr: " + getErrStr() + ", ";

      port = ntohs(((struct sockaddr_in6 *)ap->ai_addr)->sin6_port);
      out += "port: " + int2str(port) + "\n";
    }

    if (ap->ai_canonname != NULL)
      out += "  ai_canonname: " + str(ap->ai_canonname) + "\n";
    else
      out += "  ai_canonname: null\n";

    out += "  ---\n";
    ap = ap->ai_next;
  }

  std::cout << out << std::endl;
  if (ai != NULL)
    freeaddrinfo(ai);
}

// QUESTION: i think i only need one bind-wrapping function for the Server
// sockets, right? Then, it should be really clear which socket options to set.
//
// @return the fd of the socket the addr:port pair is bound to. in case of error
// -> -1
// @throws never throws exception
//
// TODO: document! especially SO_REUSEADDR
std::pair<str, int> Socket::bindSocket(const str& addr, u16 port)
{
  struct addrinfo *ai;
  int              fd;

  try {
    ai = getAddrInfo(addr, port);
  } catch (const AddrInfoException& e) {
    throw;
  }

  if (ai == NULL)
    return std::make_pair(addr, -1);

  if ((fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, ai->ai_protocol)) ==
      -1)
  {
    Logger::log_err("bindSocket socket failed: " + getErrStr());
    return std::make_pair(addr, -1);
  }

  int opt = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    Logger::log_err("bindSocket setsockopt failed: " + getErrStr());
    return std::make_pair(addr, -1);
  }

  if (bind(fd, ai->ai_addr, ai->ai_addrlen) == -1) {
    Logger::log_err("bindSocket bind failed: " + getErrStr());
    return std::make_pair(addr, -1);
  }

  freeaddrinfo(ai);

  str ipaddr = inAddrToStr(
      reinterpret_cast<struct sockaddr_in *>(ai->ai_addr)->sin_addr);

  return std::make_pair(ipaddr, fd);
}

Socket::AddrInfoException::AddrInfoException(const std::string& msg):
  std::runtime_error("AddrInfoException: " + msg)
{}
