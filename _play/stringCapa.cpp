#include <iostream>
#include <string>

int main(int ac, char **av)
{
  (void)ac, (void)av;

  std::string moep("blablabla");

  std::cout << "moep.capacity(): " << moep.capacity() << std::endl;

  moep.clear();

  std::cout << "moep.capacity(): " << moep.capacity() << std::endl;

  return 0;
}
