#include <iostream>
#include <string>

int main()
{

  std::string s;

  const char c[] = {0, 0, 0, 0, 0, 0};

  std::cout << "s.size(): " << s.size() << std::endl;
  s.append(c, sizeof(c));
  std::cout << "s.size(): " << s.size() << std::endl;

  std::string sub;
  sub.assign(s.substr(3).data(), s.size() - 3);
  std::cout << "sub.size(): " << sub.size() << std::endl;

  return 0;
}
