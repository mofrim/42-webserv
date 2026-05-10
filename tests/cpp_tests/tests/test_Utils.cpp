/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_Utils.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 13:06:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/10 22:59:54 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test-utils.hpp"
#include "test_hdr.hpp"

// test logic goes here, for any failure return (-1) or throw exception.
// otherwise -> success
int _test_Utils()
{
  print_test_topic("test_utils", "splitStrWhite");
  {
    str              s("    GET    DELETE POST   ");
    std::vector<str> sp = splitStrWhite(s);
    for (std::vector<str>::iterator it = sp.begin(); it != sp.end(); ++it)
      std::cout << "sp: '" << *it << "'" << std::endl;
  }
  print_test_topic("test_utils", "splitStrWhite");
  {
    str              s("");
    std::vector<str> sp = splitStrWhite(s);
    for (std::vector<str>::iterator it = sp.begin(); it != sp.end(); ++it)
      std::cout << "sp: '" << *it << "'" << std::endl;
  }
  return 0;
}

void test_Utils()
{
  int ret = 0;
  print_test_section_header("BEGIN utils");
  try {
    ret = _test_Utils();
  } catch (const std::exception& e) {
    print_test_section_header("END utils");
    print_test_result(false,
        "Test \"test_utils\" failed with following exception:\n" +
            std::string(e.what()));
    g_GlobalResult = KO;
    return;
  }
  if (ret == -1) {
    print_test_section_header("END utils");
    print_test_result(false, "Test \"test_utils\" failed with -1");
    g_GlobalResult = KO;
    return;
  }
  print_test_section_header("END utils");
  print_test_result(true, "Test \"test_utils\" succeeded =)");
}
