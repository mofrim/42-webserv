/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_server_init.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/08 11:40:47 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/18 16:01:49 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test_hdr.hpp"
#include "test_utils.hpp"

// test logic goes here, for any failure return (-1) or throw exception.
// otherwise -> success
int _test_server_init()
{
  print_test_topic("test_server_init", "moep");
  VServerCfg cfg;
  VServer    srv(cfg);
  return (0);
}

void test_server_init()
{
  int ret = 0;
  print_test_section_header("server_init");
  try {
    ret = _test_server_init();
  } catch (const std::exception& e) {
    print_test_result(false,
        "Test \"test_server_init\" failed with following exception:\n" +
            std::string(e.what()));
    g_GlobalResult = KO;
    return;
  }
  if (ret == -1) {
    print_test_result(false, "Test \"test_server_init\" failed with -1");
    g_GlobalResult = KO;
    return;
  }
  print_test_section_header("server_init");
  print_test_result(true, "Test \"test_server_init\" succeeded =)");
}
