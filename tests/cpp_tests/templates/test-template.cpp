/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test-template.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 13:06:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/07 07:59:24 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test_hdr.hpp"
#include "test_utils.hpp"

// test logic goes here, for any failure return (-1) or throw exception.
// otherwise -> success
int _test_template()
{
  print_test_topic("test_template", "moep");
  return (0);
}

void test_template()
{
  int ret = 0;
  print_test_section_header("BEGIN template");
  try {
    ret = _test_template();
  } catch (const std::exception& e) {
    print_test_section_header("END template");
    print_test_result(false,
        "Test \"test_template\" failed with following exception:\n" +
            std::string(e.what()));
    g_GlobalResult = KO;
    return;
  }
  if (ret == -1) {
    print_test_section_header("END template");
    print_test_result(false, "Test \"test_template\" failed with -1");
    g_GlobalResult = KO;
    return;
  }
  print_test_section_header("END template");
  print_test_result(true, "Test \"test_template\" succeeded =)");
}
