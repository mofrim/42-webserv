/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_URL.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fmaurer <fmaurer42@posteo.de>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/01/13 13:06:35 by fmaurer           #+#    #+#             */
/*   Updated: 2026/05/07 08:19:11 by fmaurer          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "test_hdr.hpp"
#include "test_utils.hpp"

struct TestCase {
    str  input;
    str  expected_output;
    bool should_throw;
};

// test logic goes here, for any failure return (-1) or throw exception.
// otherwise -> success
int _test_URL()
{
  std::vector<TestCase> test_cases;

  // path concatenation
  test_cases.push_back((TestCase){"/../bla.txt", "/bla.txt", false});
  test_cases.push_back((TestCase){"/foo/./bar", "/foo/bar", false});
  test_cases.push_back((TestCase){"/foo/../bar", "/bar", false});
  test_cases.push_back((TestCase){"/foo//bar", "/foo/bar", false});
  test_cases.push_back((TestCase){"/", "/", false});
  test_cases.push_back((TestCase){"//", "/", false});
  test_cases.push_back((TestCase){"/foo/../../bar", "/bar", false});
  test_cases.push_back((TestCase){"/foo/../../../bar", "/bar", false});
  test_cases.push_back((TestCase){"/foo.html/.", "/foo.html/", false});

  // with query strings
  test_cases.push_back((TestCase){"/foo?key=value", "/foo?key=value", false});
  test_cases.push_back((TestCase){
      "/foo?key1=value1&key2=value2", "/foo?key1=value1&key2=value2", false});
  test_cases.push_back(
      (TestCase){"/foo?key=value%201", "/foo?key=value 1", false});
  test_cases.push_back(
      (TestCase){"/foo?key=%2Fvalue", "/foo?key=/value", false});
  test_cases.push_back((TestCase){
      "/foo?key1=value1&key2=value%202",
      "/foo?key1=value1&key2=value 2",
      false});
  test_cases.push_back((TestCase){"/foo?", "/foo?", false});
  test_cases.push_back((TestCase){"/foo?key", "/foo?key", false});

  // fragments...
  test_cases.push_back((TestCase){"/foo#section", "/foo#section", false});
  test_cases.push_back(
      (TestCase){"/foo?key=value#section", "/foo?key=value#section", false});
  test_cases.push_back((TestCase){"/foo#", "/foo#", false});

  // percent encoded stuff
  test_cases.push_back((TestCase){"/foo%2Fbar", "/foo/bar", false});
  test_cases.push_back((TestCase){"/%2E%2E%2Fbar", "/bar", false});
  test_cases.push_back((TestCase){"/%2E%2E/%2E%2E/bar", "/bar", false});
  test_cases.push_back((TestCase){"/foo%20bar", "/foo bar", false});
  test_cases.push_back((TestCase){"/%41%42%43", "/ABC", false});

  // edgy cases
  test_cases.push_back((TestCase){"", "/", false});
  test_cases.push_back((TestCase){"?key=value", "/?key=value", false});
  test_cases.push_back((TestCase){"#fragment", "/?#fragment", false});
  test_cases.push_back(
      (TestCase){"/foo?key=value#fragment", "/foo?key=value#fragment", false});
  test_cases.push_back((TestCase){"/foo/bar/", "/foo/bar/", false});

  // combined stuff
  test_cases.push_back((TestCase){
      "/../foo%2Fbar?key=value%201#sec", "/foo/bar?key=value 1#sec", false});
  test_cases.push_back((TestCase){
      "/foo/./../bar//baz?k1=v1&k2=v2#frag",
      "/bar/baz?k1=v1&k2=v2#frag",
      false});
  test_cases.push_back(
      (TestCase){"/%2E%2E/foo%20bar?k=%2Fv#%23", "/foo bar?k=/v#%23", false});

  // malicious_cases
  std::vector<TestCase> malicious_cases;
  malicious_cases.push_back((TestCase){"/foo\0bar", "", true});
  malicious_cases.push_back((TestCase){"/foo?key=\0value", "", true});
  malicious_cases.push_back((TestCase){"/foo%00bar", "", true});

  print_test_topic("test_URL", "standard stuff");
  {
    URL u;
    for (std::vector<TestCase>::iterator it = test_cases.begin();
        it != test_cases.end();
        ++it)
      if (u.sanitizeTargetURL(it->input) != it->expected_output) {
        std::cout << "Test failed for input '" << it->input
                  << "', expected_output '" << it->expected_output
                  << "' -> output: '" << u.sanitizeTargetURL(it->input) << "'"
                  << std::endl;
        return -1;
      }
  }

  print_test_topic("test_URL", "malicious stuff");
  {
    URL u;
    for (std::vector<TestCase>::iterator it = malicious_cases.begin();
        it != malicious_cases.end();
        ++it)
      if (u.sanitizeTargetURL(it->input) != it->expected_output) {
        std::cout << "Test failed for input '" << it->input
                  << "', expected_output '" << it->expected_output
                  << "' -> output: '" << u.sanitizeTargetURL(it->input) << "'"
                  << std::endl;
        return -1;
      }
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
