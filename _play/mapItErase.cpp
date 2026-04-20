#include <iostream>
#include <map>
#include <string>

int main(int ac, char **av)
{
  (void)ac, (void)av;

  std::map< std::string, int> moep;

  moep["bla"]   = 12;
  moep["blub"]  = 1;
  moep["blarg"] = 2;
  moep["at"]    = 42;

  std::map< std::string, int>::iterator it = moep.begin();

  while (it != moep.end()) {
    if (it->second == 2) {
      std::map< std::string, int>::iterator itBak = it;
      it++;
      moep.erase(itBak);
      continue;
    }
    std::cout << "it(" << it->first << ") = " << it->second << std::endl;
    it++;
  }

  return 0;
}
