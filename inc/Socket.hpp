/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/28 07:26:01 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/20 13:22:23 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// Making this a utility / factory class!

#ifndef ADDR_HPP
#define ADDR_HPP

#include "typesAndConstants.hpp"

#include <netinet/in.h>
#include <stdexcept>

class Socket {
  private:
    Socket();
    Socket(const Socket& other);
    Socket& operator=(const Socket& other);
    ~Socket();

  public:
    static void                printAddrlist(const str& addr, u16 port);
    static struct addrinfo    *getAddrInfo(const str& addr, u16 port);
    static std::pair<str, int> bindSocket(const str& addr, u16 port);

    class AddrInfoException: public std::runtime_error {
      public:
        AddrInfoException(const str& msg);
    };
};

#endif
