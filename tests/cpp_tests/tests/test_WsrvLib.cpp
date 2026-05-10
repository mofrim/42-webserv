/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_WsrvLib.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 13:06:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/10 22:54:00 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test-utils.hpp"
#include "test_hdr.hpp"

// test logic goes here, for any failure return (-1) or throw exception.
// otherwise -> success
int _test_WsrvLib()
{
  print_test_topic("test_WsrvLib", "basic tests");
  {
    str moep = WsrvLib::getDefaultErrPage(HTTP_404);
    std::cout << moep << std::endl;
  }

  print_test_topic("test_WsrvLib", "WsrvSettings");
  {
    std::cout << "WsrvLib::WsrvSettings.connKeepaliveTimeout: "
              << WsrvLib::WsrvSettings.connKeepaliveTimeout << std::endl;
    std::cout << "WsrvLib::WsrvSettings.reqTimeout: "
              << WsrvLib::WsrvSettings.reqTimeout << std::endl;
    std::cout << "WsrvLib::WsrvSettings.maxClients: "
              << WsrvLib::WsrvSettings.maxClients << std::endl;
  }
  return (0);
}

void test_WsrvLib()
{
  int ret = 0;
  print_test_section_header("BEGIN WsrvLib");
  try {
    ret = _test_WsrvLib();
  } catch (const std::exception& e) {
    print_test_result(false,
        "Test \"test_WsrvLib\" failed with following exception:\n" +
            std::string(e.what()));
    g_GlobalResult = KO;
    return;
  }
  if (ret == -1) {
    print_test_result(false, "Test \"test_WsrvLib\" failed with -1");
    g_GlobalResult = KO;
    return;
  }
  print_test_section_header("END WsrvLib");
  print_test_result(true, "Test \"test_WsrvLib\" succeeded =)");
}
