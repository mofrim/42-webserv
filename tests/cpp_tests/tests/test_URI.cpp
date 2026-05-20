/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_URI.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 13:06:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/20 11:55:27 by fmaurer          ###   ########.fr       */
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
int _test_URI()
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

  print_test_topic("test_URI", "standard stuff");
  {
    for (std::vector<TestCase>::iterator it = test_cases.begin();
        it != test_cases.end();
        ++it)
    {
      URI u;
      if (u.parsePath(it->input) != it->expected_output) {
        std::cout << "Test failed for input '" << it->input
                  << "', expected_output '" << it->expected_output
                  << "' -> output: '" << u.parsePath(it->input) << "'"
                  << std::endl;
        return -1;
      }
    }
  }

  print_test_topic("test_URI", "malicious stuff");
  {
    for (std::vector<TestCase>::iterator it = maliciousCases.begin();
        it != maliciousCases.end();
        ++it)
    {
      URI u;
      if (u.parsePath(it->input) != it->expected_output) {
        std::cout << "Test failed for input '" << it->input
                  << "', expected_output '" << it->expected_output
                  << "' -> output: '" << u.parsePath(it->input) << "'"
                  << std::endl;
        return -1;
      }
    }
  }

  print_test_topic("test_URI", "other stuff");
  {
    URI u("/bla/blub/index.html");
    if (u.getPath() != "/bla/blub/index.html")
      return -1;
    u.parsePath("bla.blub.de/index.html");
    if (u.getPath() != "")
      return -1;
    u.parsePath("?/index.html");
    if (u.getPath() != "")
      return -1;
    u.parsePath("////index.html");
    std::cout << "u.getPath(): " << u.getPath() << std::endl;
    if (u.getPath() != "/index.html")
      return -1;
  }

  print_test_topic("test_URI", "real URI parsing!");
  {
    URI u;
    if (u.parseURL("/") != "" && !u.bad())
      return -1;
    if (u.parseURL("//") != "" && !u.bad())
      return -1;
    if (u.parseURL("") != "" && !u.bad())
      return -1;
    if (u.parseURL("http") != "" && !u.bad())
      return -1;
    if (u.parseURL("http//") != "" && !u.bad())
      return -1;
    if (u.parseURL("http://") != "" && !u.bad())
      return -1;
    if (u.parseURL("http:///") != "" && !u.bad())
      return -1;
    if (u.parseURL("http://a/") == "" && u.bad())
      return -1;
    if (u.parseURL("http://asdasldj.asd.asd.a/") == "" && u.bad())
      return -1;
    if (u.parseURL("http://asdasldj..asd.asd.a/") != "" && u.bad())
      return -1;
    if (u.parseURL("http://hallo-./") != "" && u.bad())
      return -1;
    if (u.parseURL("http://./") != "" && u.bad())
      return -1;
    if (u.parseURL("http://this.that.and.those./") != "" && u.bad())
      return -1;
    if (u.parseURL("http://surbelkarg:1279172971239723/") != "" || !u.bad())
      return -1;
    if (u.parseURL("http://surbelkarg:123/") == "" || u.bad())
      return -1;
    if (u.parseURL("http://s:1/") == "" || u.bad())
      return -1;
    if (u.parseURL("http://s:/") != "" || !u.bad())
      return -1;
    if (u.parseURL("http://s:65535/") == "" || u.bad())
      return -1;
    if (u.parseURL("http://s:65536/") != "" || !u.bad())
      return -1;
    if (u.parseURL("http://:/") != "" || !u.bad())
      return -1;
    if (u.parseURL("http://hkahdsad.:213/") != "" || !u.bad())
      return -1;
    if (u.parseURL("http://   :213/") != "" || !u.bad())
      return -1;
    if (u.parseURL("http://adshhasd%:213/") != "" || !u.bad())
      return -1;
    if (u.parseURL("http://adshhasd:213") == "" && u.getPort() != 213)
      return -1;
    if (u.parseURL("http://adshhasd:as213") != "")
      return -1;
    if (u.parseURL("http://adshhasd:213") == "" && u.getHost() != "adshhasd")
      return -1;
    if (u.parseURL("http://adshhasd") == "" && u.getHost() != u.getAuth())
      return -1;
    if (u.parseURL("http://adshhasd:123") == "" && u.getHost() != u.getAuth())
      return -1;
    if (u.parseURL("http://adshhasd:123") == "" && u.getPort() != 123)
      return -1;
    if (u.parseURL("http://adshhasd:0") != "")
      return -1;
  }

  print_test_topic("test_URI", "getStr()");
  {
    URI u("http://42.fr:4231/path/to/sth?jhasdh=jashdash");
    if (u.bad())
      std::cout << "bad" << std::endl;
    std::cout << "u.getStr(): " << u.getStr() << std::endl;

    u.parseURL("http://42.fr:4231/path/to/sth?jhasdh=jashdash&miep=moep");
    if (u.bad())
      std::cout << "bad" << std::endl;
    std::cout << "u.getStr(): " << u.getStr() << std::endl;
  }

  return 0;
}

void test_URI()
{
  int ret = 0;
  print_test_section_header("BEGIN URI");
  try {
    ret = _test_URI();
  } catch (const std::exception& e) {
    print_test_section_header("END URI");
    print_test_result(false,
        "Test \"test_URI\" failed with following exception:\n" +
            std::string(e.what()));
    g_GlobalResult = KO;
    return;
  }
  if (ret == -1) {
    print_test_section_header("END URI");
    print_test_result(false, "Test \"test_URI\" failed with -1");
    g_GlobalResult = KO;
    return;
  }
  print_test_section_header("END URI");
  print_test_result(true, "Test \"test_URI\" succeeded =)");
}
