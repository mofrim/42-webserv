/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_URL.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 13:06:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/11 00:22:20 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test-utils.hpp"
#include "test_hdr.hpp"

struct TestCase {
    str input;
    str expected_output;
};

// test logic goes here, for any failure return (-1) or throw exception.
// otherwise -> success
int _test_URL()
{
  std::vector<TestCase> test_cases;

  // path concatenation
  test_cases.push_back((TestCase){"/../bla.txt", "/bla.txt"});
  test_cases.push_back((TestCase){"/foo/./bar", "/foo/bar"});
  test_cases.push_back((TestCase){"/foo/../bar", "/bar"});
  test_cases.push_back((TestCase){"/foo//bar", "/foo/bar"});
  test_cases.push_back((TestCase){"/", "/"});
  test_cases.push_back((TestCase){"//", "/"});
  test_cases.push_back((TestCase){"/foo/../../bar", "/bar"});
  test_cases.push_back((TestCase){"/foo/../../../bar", "/bar"});
  test_cases.push_back((TestCase){"/foo.html/.", "/foo.html/"});
  test_cases.push_back((TestCase){"/foo.html/...", "/foo.html/.../"});

  // with query strings
  test_cases.push_back((TestCase){"/foo?key=value", "/foo?key=value"});
  test_cases.push_back((TestCase){
      "/foo?key1=value1&key2=value2", "/foo?key1=value1&key2=value2"});
  test_cases.push_back((TestCase){"/foo?key=value%201", "/foo?key=value 1"});
  test_cases.push_back((TestCase){"/foo?key=%2Fvalue", "/foo?key=/value"});
  test_cases.push_back((TestCase){
      "/foo?key1=value1&key2=value%202",
      "/foo?key1=value1&key2=value 2",
  });
  test_cases.push_back((TestCase){"/foo?", "/foo"});
  test_cases.push_back((TestCase){"/foo?key", "/foo?key"});

  // fragments...
  test_cases.push_back((TestCase){"/foo#section", "/foo#section"});
  test_cases.push_back(
      (TestCase){"/foo?key=value#section", "/foo?key=value#section"});
  test_cases.push_back((TestCase){"/foo#", "/foo#"});

  // percent encoded stuff
  test_cases.push_back((TestCase){"/foo%2Fbar", "/foo/bar"});
  test_cases.push_back((TestCase){"/%2E%2E%2Fbar", "/bar"});
  test_cases.push_back((TestCase){"/%2E%2E/%2E%2E/bar", "/bar"});
  test_cases.push_back((TestCase){"/foo%20bar", "/foo bar"});
  test_cases.push_back((TestCase){"/%41%42%43", "/ABC"});

  // edgy cases
  test_cases.push_back((TestCase){"", "/"});
  test_cases.push_back((TestCase){"?key=value", ""});
  test_cases.push_back((TestCase){"#fragment", ""});
  test_cases.push_back(
      (TestCase){"/foo?key=value#fragment", "/foo?key=value#fragment"});
  test_cases.push_back((TestCase){"/foo/bar/", "/foo/bar/"});

  // combined stuff
  test_cases.push_back((TestCase){
      "/../foo%2Fbar?key=value%201#sec", "/foo/bar?key=value 1#sec"});
  test_cases.push_back((TestCase){
      "/foo/./../bar//baz?k1=v1&k2=v2#frag", "/bar/baz?k1=v1&k2=v2#frag"});
  test_cases.push_back(
      (TestCase){"/%2E%2E/foo%20bar?k=%2Fv#%23", "/foo bar?k=/v#%23"});

  // malicious_cases
  // TODO: make up more of this
  std::vector<TestCase> maliciousCases;
  maliciousCases.push_back((TestCase){"/foo%00bar", ""});

  print_test_topic("test_URL", "standard stuff");
  {
    for (std::vector<TestCase>::iterator it = test_cases.begin();
        it != test_cases.end();
        ++it)
    {
      URL u;
      if (u.parseTargetURL(it->input) != it->expected_output) {
        std::cout << "Test failed for input '" << it->input
                  << "', expected_output '" << it->expected_output
                  << "' -> output: '" << u.parseTargetURL(it->input) << "'"
                  << std::endl;
        return -1;
      }
    }
  }

  print_test_topic("test_URL", "malicious stuff");
  {
    for (std::vector<TestCase>::iterator it = maliciousCases.begin();
        it != maliciousCases.end();
        ++it)
    {
      URL u;
      if (u.parseTargetURL(it->input) != it->expected_output) {
        std::cout << "Test failed for input '" << it->input
                  << "', expected_output '" << it->expected_output
                  << "' -> output: '" << u.parseTargetURL(it->input) << "'"
                  << std::endl;
        return -1;
      }
    }
  }

  print_test_topic("test_URL", "other stuff");
  {
    URL u("/bla/blub/index.html");
    if (u.getPath() != "/bla/blub/index.html")
      return -1;
    u.parseTargetURL("bla.blub.de/index.html");
    if (u.getPath() != "")
      return -1;
    u.parseTargetURL("?/index.html");
    if (u.getPath() != "")
      return -1;
    u.parseTargetURL("////index.html");
    std::cout << "u.getPath(): " << u.getPath() << std::endl;
    if (u.getPath() != "/index.html")
      return -1;
  }
  return 0;
}

void test_URL()
{
  int ret = 0;
  print_test_section_header("BEGIN URL");
  try {
    ret = _test_URL();
  } catch (const std::exception& e) {
    print_test_section_header("END URL");
    print_test_result(false,
        "Test \"test_URL\" failed with following exception:\n" +
            std::string(e.what()));
    g_GlobalResult = KO;
    return;
  }
  if (ret == -1) {
    print_test_section_header("END URL");
    print_test_result(false, "Test \"test_URL\" failed with -1");
    g_GlobalResult = KO;
    return;
  }
  print_test_section_header("END URL");
  print_test_result(true, "Test \"test_URL\" succeeded =)");
}
