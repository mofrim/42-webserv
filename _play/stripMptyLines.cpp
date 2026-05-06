#include <iostream>
#include <string>
#include <vector>

#define CRLF "\r\n"
#define CRLFX2 "\r\n\r\n"

typedef std::string str;
typedef short       u16;

std::vector<str> splitString(
    const str& sstr, const str& delim, bool keepEmpty = false)
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
    if (!sub.empty() || keepEmpty)
      ret.push_back(sub);
    i = k + delim.size();
  }
  return ret;
}

size_t skipEmptyHdrLines(const str& s)
{
  size_t i     = 0;
  size_t diff  = 2;
  size_t count = 0;
  size_t next  = 0;

  while (i != str::npos) {
    next = s.find(CRLF, i + 1);
    if (next == str::npos) {
      std::cout << "doing it" << std::endl;
      if (i + 1 < s.size() && !s.compare(i, 2, CRLF)) {
        count++;
      }
      break;
    }
    diff = next - i;
    if (diff != 2 && i == 0)
      break;
    count++;
    i = next;
  }
  return count * 2;
}

size_t skip2(const str& s)
{
  std::vector<str> v = splitString(s, CRLF, true);
  std::cout << "v.size(): " << v.size() << std::endl;
  size_t i = 0;
  while (v[i].empty())
    i++;
  return i * 2;
}

int main()
{
  str teststr =
      "\r\n"
      "\r\n"
      "\r\n"
      "\r\n"
      "GET / HTTP/1.1\r\n"
      "Host: localhost:2222\r\n"
      "User-Agent: curl/8.18.0\r\n"
      "Accept: */*\r\n"
      "\r\n";

  // str teststr = "";

  // size_t skip = skipEmptyHdrLines(teststr);
  size_t skip = skip2(teststr);
  std::cout << "should skip: " << skip << ", result:\n'''" << std::endl;
  std::cout << teststr.substr(skip);
  std::cout << "'''" << std::endl;

  return 0;
}
