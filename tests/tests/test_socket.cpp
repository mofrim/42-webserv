/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_socket.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 13:06:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/18 00:29:30 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test_hdr.hpp"
#include "test_utils.hpp"

#include <iostream>
#include <stdexcept>

// test logic goes here, for any failure return (-1) or throw exception.
// otherwise -> success
int _test_socket()
{
  int ret = 0;

  Socket::printAddrlist("localhost", 1234);

  // should fail
  try {
    Socket::getAddrInfo("", 1234);
    ret = -1;
  } catch (const Socket::AddrInfoException& e) {
  }

  int r = -1;
  try {
    r = Socket::bindSocket("", 1234);
  } catch (const Socket::AddrInfoException& e) {
    std::cout << "caught exception: " << e.what() << std::endl;
    r = 0;
  }
  if (r == -1)
    ret = -1;

  return ret;
}

void test_socket()
{
  int ret = 0;
  print_test_section_header("Socket test section header");
  try {
    ret = _test_socket();
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
