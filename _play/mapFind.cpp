#include <iostream>
#include <map>
#include <string>

int main(int ac, char **av)
{
  (void)ac, (void)av;

  std::multimap<int, std::string> m;
  m.insert(std::pair<int, std::string>(1, "eins"));
  m.insert(std::pair<int, std::string>(2, "zwei"));
  m.insert(std::pair<int, std::string>(1, "nochmal eins"));

  size_t cnt = m.count(1);

  std::map<int, std::string>::iterator it = m.find(1);
  std::cout << "it->second: " << it->second << std::endl;
  cnt--, it++;
  while (cnt > 0 && it != m.end()) {
    if (it->first == 1) {
      std::cout << "it->second: " << it->second << std::endl;
      cnt--;
    }
    it++;
  }

  return 0;
}
