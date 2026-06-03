#include <iostream>
#include <string>
#include <vector>

int main(int ac, char **av)
{
  (void)ac, (void)av;

  std::string       s;
  std::vector<char> v;

  std::cout << "s.max_size(): " << s.max_size() << std::endl;
  std::cout << "v.max_size(): " << v.max_size() << std::endl;

  return 0;
}
