/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/28 07:26:01 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/17 23:21:05 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ADDR_HPP
#define ADDR_HPP

#include <netinet/in.h>
#include <stdexcept>
#include <stdint.h>
#include <string>

class Socket {
  private:
    Socket();
    Socket(const Socket& other);
    Socket& operator=(const Socket& other);

    const std::string _addr;
    const int         _port;

    struct addrinfo *_addrllist;

    // int _fd;

  public:
    Socket(const std::string& addr, uint16_t port);
    ~Socket();

    void printAddrlist() const;
    void getAddrInfo();
    void bind();

    class AddrInfoException: public std::runtime_error {
      public:
        AddrInfoException(const std::string& msg);
    };
};

#endif
