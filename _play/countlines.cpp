#include <algorithm>
#include <iostream>
#include <string>

#define CRLF "\r\n"
#define CRLFX2 "\r\n\r\n"

typedef std::string str;
typedef short       u16;

short countCRLF(const str& s)
{
  u16 lineNum = 0;

  size_t i     = 0;
  size_t ssize = s.size();

  while (i < ssize) {
    i = s.find(CRLF, i);
    if (i == str::npos)
      break;
    if ((i != str::npos && i + 3 < ssize && s.compare(i, 4, CRLFX2) == 0)) {
      lineNum += 2;
      break;
    }
    lineNum++;
    i += 2;
  }
  return lineNum;
}

int main(int ac, char **av)
{
  (void)ac, (void)av;

  std::string teststr =
      "GET / HTTP/1.1\r\n"
      "Host: localhost:2222\r\n"
      "User-Agent: curl/8.18.0\r\n"
      "Accept: */*\r\n"
      "\r\n";

  std::cout << "num of lines: " << countCRLF(teststr) << std::endl;

  teststr = "asjdljasldjasjdlj";
  std::cout << "num of lines: " << countCRLF(teststr) << std::endl;

  teststr = "";
  std::cout << "num of lines: " << countCRLF(teststr) << std::endl;

  teststr = "\r\n";
  std::cout << "num of lines: " << countCRLF(teststr) << std::endl;

  return 0;
}
