/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_ServerCfg.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 13:06:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/04/18 13:52:30 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test_hdr.hpp"
#include "test_utils.hpp"

#include <list>
#include <map>

// test logic goes here, for any failure return (-1) or throw exception.
// otherwise -> success
int _test_ServerCfg()
{
  print_test_topic("test_ServerCfg", "empty tests");
  {
    ServerCfg cfg;
    cfg.addInterface("moep.de", 1234);
    cfg.addInterface("moep.de", 1234);
    cfg.addInterface("moep.de", 1235);

    std::map< str, std::set<u16> >           ifs = cfg.getInterfaces();
    std::map< str, std::set<u16> >::iterator it  = ifs.begin();
    for (; it != ifs.end(); it++)
      std::cout << "addr: " << it->first << ", port(s): " << it->second;
    std::cout << std::endl;
  }
  return 0;
}

void test_ServerCfg()
{
  int ret = 0;
  print_test_section_header("BEGIN ServerCfg");
  try {
    ret = _test_ServerCfg();
  } catch (const std::exception& e) {
    print_test_result(false,
        "Test \"test_ServerCfg\" failed with following exception:\n" +
            std::string(e.what()));
    g_GlobalResult = KO;
    return;
  }
  if (ret == -1) {
    print_test_result(false, "Test \"test_ServerCfg\" failed with -1");
    g_GlobalResult = KO;
    return;
  }
  print_test_section_header("END ServerCfg");
  print_test_result(true, "Test \"test_ServerCfg\" succeeded =)");
}
