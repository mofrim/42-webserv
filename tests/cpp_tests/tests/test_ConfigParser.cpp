/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test-ConfigParser.cpp                                  :+:      :+:    :+:
 */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 13:06:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/07 07:59:24 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test-utils.hpp"
#include "test_hdr.hpp"

// test logic goes here, for any failure return (-1) or throw exception.
// otherwise -> success
int _test_ConfigParser()
{
  print_test_topic("test_ConfigParser", "check knownDirectives");
  {
    ConfigParser  c("tmp");
    std::set<str> s = c.getKnownDirectives();
    for (std::set<str>::const_iterator it = s.begin(); it != s.end(); ++it)
      std::cout << *it << ", " << std::endl;
  }

  print_test_topic("test_ConfigParser", "iteration test!!");
  {
    ConfigParser p("tests/parsing/default.wsrv");
    if (p.bad())
      return -1;
    p.parse();
    std::vector<VServerCfg> v = p.getCfgs();
    std::cout << "v.size = " << v.size() << std::endl;
    if (v.size() > 0)
      v[0].printCfg();
  }
  return 0;
}

void test_ConfigParser()
{
  int ret = 0;
  print_test_section_header("BEGIN ConfigParser");
  try {
    ret = _test_ConfigParser();
  } catch (const std::exception& e) {
    print_test_section_header("END ConfigParser");
    print_test_result(false,
        "Test \"test_ConfigParser\" failed with following exception:\n  " RED +
            std::string(e.what()));
    g_GlobalResult = KO;
    return;
  }
  if (ret == -1) {
    print_test_section_header("END ConfigParser");
    print_test_result(false, "Test \"test_ConfigParser\" failed with -1");
    g_GlobalResult = KO;
    return;
  }
  print_test_section_header("END ConfigParser");
  print_test_result(true, "Test \"test_ConfigParser\" succeeded =)");
}
