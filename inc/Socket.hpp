/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/28 07:26:01 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/17 20:53:38 by fmaurer          ###   ########.fr       */
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

    // pointer to linked list holding the addrinfo after successfull call to
    // ctor
    struct addrinfo *_addrllist;

    const std::string _addr;
    const int         _port;

  public:
    Socket(const std::string& addr, uint16_t port);
    ~Socket();

    void print_addrllist() const;

    class AddrInfoException: public std::runtime_error {
      public:
        AddrInfoException(const std::string& msg);
    };
};

#endif
