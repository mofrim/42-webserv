/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_VServerCfg.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 13:06:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/10 22:54:22 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test-utils.hpp"
#include "test_hdr.hpp"

#include <map>

// test logic goes here, for any failure return (-1) or throw exception.
// otherwise -> success
int _test_VServerCfg()
{
  print_test_topic("test_ServerCfg", "double ports test");
  {
    VServerCfg cfg;
    cfg.addInterface("moep.de", 1234);
    cfg.addInterface("moep.de", 1234);
    cfg.addInterface("moep.de", 1235);

    std::map< str, std::set<u16> >           ifs = cfg.getInterfaces();
    std::map< str, std::set<u16> >::iterator it  = ifs.begin();
    for (; it != ifs.end(); it++)
      std::cout << "addr: " << it->first << ", port(s): " << it->second;
    std::cout << std::endl;
  }

  print_test_topic("test_ServerCfg", "multiple addrs");
  {
    VServerCfg cfg;

    std::map< str, std::set<u16> > lst;

    lst["10.0.0.1"].insert(0);
    lst["10.0.0.1"].insert(1);
    lst["10.0.0.1"].insert(2);
    lst["10.0.0.2"].insert(3);
    lst["10.0.0.3"];

    cfg.addInterfaces(lst);

    std::map< str, std::set<u16> >::const_iterator it =
        cfg.getInterfaces().begin();
    for (; it != cfg.getInterfaces().end(); it++) {
      std::cout << "addr: " << it->first << ", port(s): " << it->second;
      std::cout << std::endl;
    }
  }

  print_test_topic("test_VServerCfg", "assignment operator");
  {
    VServerCfg cfg0;

    std::map< str, std::set<u16> > lst;

    lst["10.0.0.1"].insert(0);
    lst["10.0.0.1"].insert(1);
    lst["10.0.0.1"].insert(2);
    lst["10.0.0.2"].insert(3);
    lst["10.0.0.3"];
    cfg0.addInterfaces(lst);

    VServerCfg cfg1 = cfg0;

    std::map< str, std::set<u16> >::const_iterator it =
        cfg1.getInterfaces().begin();
    for (; it != cfg1.getInterfaces().end(); it++) {
      std::cout << "addr: " << it->first << ", port(s): " << it->second;
      std::cout << std::endl;
    }
  }

  print_test_topic("test_ServerCfg", "delete");
  {
    VServerCfg cfg;

    std::map< str, std::set<u16> > lst;

    lst["10.0.0.1"].insert(0);
    lst["10.0.0.1"].insert(1);
    lst["10.0.0.1"].insert(2);
    lst["10.0.0.2"].insert(3);
    lst["10.0.0.3"];

    cfg.addInterfaces(lst);

    cfg.delInterface("10.0.0.1");

    std::map< str, std::set<u16> >::const_iterator it =
        cfg.getInterfaces().begin();
    for (; it != cfg.getInterfaces().end(); it++) {
      std::cout << "addr: " << it->first << ", port(s): " << it->second;
      std::cout << std::endl;
    }
  }

  print_test_topic("test_VServerCfg", "delete all");
  {
    VServerCfg cfg;

    std::map< str, std::set<u16> > lst;

    lst["10.0.0.1"].insert(0);
    lst["10.0.0.1"].insert(1);
    lst["10.0.0.1"].insert(2);
    lst["10.0.0.2"].insert(3);
    lst["10.0.0.3"];

    cfg.addInterfaces(lst);

    if (cfg.delInterface("10.0.0.1") == -1)
      return -1;
    if (cfg.delInterface("10.0.0.2") == -1)
      return -1;
    if (cfg.delInterface("10.0.0.3") == -1)
      return -1;

    std::map< str, std::set<u16> >::const_iterator it =
        cfg.getInterfaces().begin();
    for (; it != cfg.getInterfaces().end(); it++) {
      std::cout << "addr: " << it->first << ", port(s): " << it->second;
      std::cout << std::endl;
    }
  }
  return 0;
}

void test_VServerCfg()
{
  int ret = 0;
  print_test_section_header("BEGIN VServerCfg");
  try {
    ret = _test_VServerCfg();
  } catch (const std::exception& e) {
    print_test_result(false,
        "Test \"test_ServerCfg\" failed with following exception:\n" +
            std::string(e.what()));
    g_GlobalResult = KO;
    return;
  }
  if (ret == -1) {
    print_test_result(false, "Test \"test_VServerCfg\" failed with -1");
    g_GlobalResult = KO;
    return;
  }
  print_test_section_header("END VServerCfg");
  print_test_result(true, "Test \"test_VServerCfg\" succeeded =)");
}
