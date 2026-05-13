/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_Routes.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 13:06:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/13 23:24:45 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test-utils.hpp"
#include "test_hdr.hpp"

// test logic goes here, for any failure return (-1) or throw exception.
// otherwise -> success
int _test_Routes()
{
  print_test_topic("test_Routes", "moep");
  {
    std::vector<VServer> dummy;
    Route                r;

    r.setAutoindex(true);
    r.setIndex("moep.html");
    r.setPath("/miep");
    r.setRoot("./WWW");

    VServerCfg cfg;
    cfg.addRoute(r);
    cfg.setServerName("Moep.de");
    cfg.addInterface("127.0.0.1", 2222);

    VServer s(cfg);
    s.init(dummy.begin(), dummy.begin());
    s.printCfg();
  }

  print_test_topic("test_Routes", "overwrite");
  {
    std::vector<VServer> dummy;
    Route                r;

    r.setAutoindex(true);
    r.setIndex("moep.html");
    r.setPath("/miep");
    r.setRoot("./WWW");

    Route r1;

    r1.setAutoindex(true);
    r1.setIndex("ha, gotcha!");
    r1.setPath("/miep");
    r1.setRoot("./overwrite");

    VServerCfg cfg;
    cfg.addRoute(r);
    cfg.addRoute(r1);
    cfg.setServerName("Moep.de");
    cfg.addInterface("127.0.0.1", 2222);

    VServer s(cfg);
    s.init(dummy.begin(), dummy.begin());
    s.printCfg();
  }

  print_test_topic("test_Routes", "root");
  {
    Route r;
    r.setRoot("////");
    if (r.getRoot() != "")
      return -1;
    r.setRoot("moep/");
    if (r.getRoot() != "moep")
      return -1;
    r.setRoot("moep///");
    if (r.getRoot() != "moep")
      return -1;
    r.setRoot("/moep");
    if (r.getRoot() != "/moep")
      return -1;
    r.setRoot("///moep");
    if (r.getRoot() != "///moep")
      return -1;
  }

  return 0;
}

void test_Routes()
{
  int ret = 0;
  print_test_section_header("BEGIN Routes");
  try {
    ret = _test_Routes();
  } catch (const std::exception& e) {
    print_test_result(false,
        "Test \"test_Routes\" failed with following exception:\n" +
            std::string(e.what()));
    g_GlobalResult = KO;
    return;
  }
  if (ret == -1) {
    print_test_result(false, "Test \"test_Routes\" failed with -1");
    g_GlobalResult = KO;
    return;
  }
  print_test_section_header("END Routes");
  print_test_result(true, "Test \"test_Routes\" succeeded =)");
}
