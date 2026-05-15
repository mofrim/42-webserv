#include <iostream>
#include <map>
#include <string>

int main(int ac, char **av)
{
  (void)ac, (void)av;

  std::map<int, std::string> smap;

  smap[100] = "dont";
  smap[30]  = "panic";
  smap[101] = "i";
  smap[1]   = "said";
  smap[12]  = "said";

  for (std::map<int, std::string>::iterator it = smap.begin(); it != smap.end();
      ++it)
  {
    std::cout << "key: " << it->first << std::endl;
  }

  return 0;
}
