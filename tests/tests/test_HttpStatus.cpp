/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_HttpStatus.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 13:06:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/23 18:59:24 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test_hdr.hpp"
#include "test_utils.hpp"

// test logic goes here, for any failure return (-1) or throw exception.
// otherwise -> success
int _test_HttpStatus()
{
  print_test_topic("test_HttpStatus", "basic tests");

  str moep = HttpStatus::getDefaultErrPage(404);
  std::cout << moep << std::endl;
  return (0);
}

void test_HttpStatus()
{
  int ret = 0;
  print_test_section_header("BEGIN HttpStatus");
  try {
    ret = _test_HttpStatus();
  } catch (const std::exception& e) {
    print_test_result(false,
        "Test \"test_HttpStatus\" failed with following exception:\n" +
            std::string(e.what()));
    g_GlobalResult = KO;
    return;
  }
  if (ret == -1) {
    print_test_result(false, "Test \"test_HttpStatus\" failed with -1");
    g_GlobalResult = KO;
    return;
  }
  print_test_section_header("END HttpStatus");
  print_test_result(true, "Test \"test_HttpStatus\" succeeded =)");
}
