#include <iostream>
#include <string>
#include <vector>
typedef std::string str;

std::vector<str> splitStrWhite(const str& sstr, bool keepEmpty = false)
{
  std::vector<str> ret;
  size_t           i = 0;
  size_t           k = 0;

  const str ws(" \t\v\f\n");

  if (sstr.empty())
    return ret;

  i = sstr.find_first_not_of(ws);
  if (i == str::npos)
    return ret;

  while (i < sstr.size() && k != str::npos) {
    k = sstr.find_first_of(ws, i);
    str sub;
    if (k == str::npos)
      sub = sstr.substr(i, str::npos);
    else
      sub = sstr.substr(i, k - i);
    if (!sub.empty() || keepEmpty)
      ret.push_back(sub);
    while (k < sstr.size() && isspace(sstr[k]))
      ++k;
    i = k;
  }
  return ret;
}

int main(int ac, char **av)
{
  (void)ac, (void)av;

  str s("/usr/bin/bash");
  std::cout << "splitStrWhite of '" << s << "':" << std::endl;
  std::vector<str> sp = splitStrWhite(s);
  for (std::vector<str>::iterator it = sp.begin(); it != sp.end(); ++it)
    std::cout << "sp: '" << *it << "'" << std::endl;

  s = "/usr/bin/env bash";
  std::cout << "splitStrWhite of '" << s << "':" << std::endl;
  sp = splitStrWhite(s);
  for (std::vector<str>::iterator it = sp.begin(); it != sp.end(); ++it)
    std::cout << "sp: '" << *it << "'" << std::endl;

  s = "miep\tmoep\n";
  std::cout << "splitStrWhite of '" << s << "':" << std::endl;
  sp = splitStrWhite(s);
  for (std::vector<str>::iterator it = sp.begin(); it != sp.end(); ++it)
    std::cout << "sp: '" << *it << "'" << std::endl;

  s = "  \t\n\v\f";
  std::cout << "splitStrWhite of '" << s << "':" << std::endl;
  sp = splitStrWhite(s);
  for (std::vector<str>::iterator it = sp.begin(); it != sp.end(); ++it)
    std::cout << "sp: '" << *it << "'" << std::endl;

  return 0;
}
