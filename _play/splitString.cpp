#include <iostream>
#include <sstream>
#include <string>
#include <vector>

typedef std::string str;

#define CRLF "\r\n"

std::vector<str> splitString(const str& sstr, const str& delim)
{
  std::vector<str> ret;
  size_t           i = 0;
  size_t           k = 0;

  if (sstr.empty() || delim.empty()) {
    ret.push_back(sstr);
    return ret;
  }

  while (i < sstr.size() && k != str::npos) {
    k = sstr.find(delim, i);
    str sub;
    if (k == str::npos)
      sub = sstr.substr(i, str::npos);
    else
      sub = sstr.substr(i, k - i);
    if (!sub.empty())
      ret.push_back(sub);
    i = k + delim.size();
  }
  return ret;
}

int main(int ac, char **av)
{
  (void)ac, (void)av;

  str teststr =
      "GET / HTTP/1.1\r\n"
      "Host: localhost:2222\r\n"
      "User-Agent: curl/8.18.0\r\n"
      "Accept: */*\r\n"
      "\r\n";

  std::vector<str> sdf = splitString(teststr, CRLF);

  for (size_t i = 0; i < sdf.size(); i++)
    std::cout << "sdf[i]: " << sdf[i] << std::endl;

  // std::cout << "asjkdalsj" << std::endl;
  //
  // std::vector<str> moep;
  // for (size_t i = 0; i < sdf.size(); i++) {
  //   std::vector<str> miep = splitString(sdf[i], ": ");
  //   for (size_t k = 0; k < miep.size(); k++)
  //     moep.push_back(miep[k]);
  // }
  //
  // for (size_t k = 0; k < moep.size(); k++)
  //   std::cout << "moep[k]: " << moep[k] << std::endl;

  {
    std::vector<str> asd = splitString("asdjaklsjd: atat", ": ");
    for (size_t i = 0; i < asd.size(); i++)
      std::cout << "asd[i]: " << asd[i] << std::endl;
  }
  std::cout << std::endl;

  std::cout << "------------------" << std::endl;

  {
    std::vector<str> asd = splitString(": ", ": ");
    for (size_t i = 0; i < asd.size(); i++)
      std::cout << "asd[i]: " << asd[i] << std::endl;
  }
  std::cout << std::endl;

  std::cout << "------------------" << std::endl;
  std::cout << "string by emptyu" << std::endl;
  std::cout << "------------------" << std::endl;

  {
    std::vector<str> asd = splitString("asbdas", "");
    for (size_t i = 0; i < asd.size(); i++)
      std::cout << "asd[i]: " << asd[i] << std::endl;
  }
  std::cout << std::endl;

  std::cout << "------------------" << std::endl;
  std::cout << "string by space" << std::endl;
  std::cout << "------------------" << std::endl;

  {
    std::vector<str> asd = splitString("a s b d a s", " ");
    for (size_t i = 0; i < asd.size(); i++)
      std::cout << "asd[i]: " << asd[i] << std::endl;
  }
  std::cout << std::endl;

  std::cout << "------------------" << std::endl;
  std::cout << "space" << std::endl;
  std::cout << "------------------" << std::endl;

  {
    std::vector<str> asd = splitString(" ", " ");
    for (size_t i = 0; i < asd.size(); i++)
      std::cout << "asd[i]: " << asd[i] << std::endl;
  }
  std::cout << std::endl;

  std::cout << "------------------" << std::endl;
  std::cout << "empty by empty" << std::endl;
  std::cout << "------------------" << std::endl;

  {
    std::vector<str> asd = splitString("", "");
    for (size_t i = 0; i < asd.size(); i++)
      std::cout << "asd[i]: " << asd[i] << std::endl;
  }
  std::cout << std::endl;

  std::cout << "------------------" << std::endl;
  std::cout << "delim not there" << std::endl;
  std::cout << "------------------" << std::endl;

  {
    std::vector<str> asd = splitString("a,b,c,d", ";");
    for (size_t i = 0; i < asd.size(); i++)
      std::cout << "asd[i]: " << asd[i] << std::endl;
  }

  std::cout << "------------------" << std::endl;
  std::cout << "------------------" << std::endl;

  std::cout << "'" << teststr << "'" << std::endl;
  std::cout << "------------------" << std::endl;
  std::cout << "'" << teststr.substr(0, teststr.find("\r\n\r\n") + 2) << "'"
            << std::endl;

  str bla = teststr.substr(0, teststr.find("\r\n\r\n") + 2);

  std::vector<str> blub = splitString(bla, CRLF);

  std::cout << "------------------" << std::endl;
  for (size_t i = 0; i < blub.size(); i++)
    std::cout << "blub[i]: " << blub[i] << std::endl;

  return 0;
}
