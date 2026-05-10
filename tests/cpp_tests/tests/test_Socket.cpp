/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_Socket.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 13:06:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/10 22:54:18 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test-utils.hpp"
#include "test_hdr.hpp"

#include <iostream>

// test logic goes here, for any failure return (-1) or throw exception.
// otherwise -> success
int _test_Socket()
{
  print_test_topic("test_Socket", "printAddrlist");
  {
    Socket::printAddrlist("localhost", 1234);
  }

  print_test_topic("test_Socket", "getAddrInfo for empty host");
  {
    int r = -1;
    try {
      r = Socket::bindSocket("", 1234).fd;
      r = -42;
    } catch (const Socket::AddrInfoException& e) {
      std::cout << "caught exception: " << e.what() << std::endl;
    }
    if (r == -42)
      return -1;
  }

  print_test_topic("test_Socket", "getIP");
  {
    str host("localhost");
    str ip = Socket::resolveAddr(host);
    std::cout << host << ": " << ip << std::endl;
    host = "spock";
    ip   = Socket::resolveAddr(host);
    std::cout << host << ": " << ip << std::endl;
    host = "surbelkarg";
    ip   = Socket::resolveAddr(host);
    std::cout << host << ": " << ip << std::endl;
    host = "10.0.0.1";
    ip   = Socket::resolveAddr(host);
    std::cout << host << ": " << ip << std::endl;
    host = "127.0.0.1";
    ip   = Socket::resolveAddr(host);
    std::cout << host << ": " << ip << std::endl;
  }
  return 0;
}

void test_Socket()
{
  int ret = 0;
  print_test_section_header("Socket test section header");
  try {
    ret = _test_Socket();
  } catch (const std::exception& e) {
    print_test_result(false,
        "Test \"Socket\" failed with following exception:\n" +
            std::string(e.what()));
    g_GlobalResult = KO;
    return;
  }
  if (ret == -1) {
    print_test_result(false, "Test \"Socket\" failed with -1");
    g_GlobalResult = KO;
    return;
  }
  print_test_section_header("Socket");
  print_test_result(true, "Test \"Socket\" succeeded =)");
}
