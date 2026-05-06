#include <iostream>
#include <string>

int main(int ac, char **av)
{
  (void)ac, (void)av;

  std::string s;

  std::cout << "s.max_size(): " << s.max_size() << std::endl;

  return 0;
}
