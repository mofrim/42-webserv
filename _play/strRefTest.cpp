#include <iostream>
#include <string>

int main(int ac, char **av)
{
  (void)ac, (void)av;
  std::string  moep = "moep";
  std::string& ref  = moep;

  std::cout << "ref: " << ref << std::endl;

  moep = "miep";

  std::cout << "ref: " << ref << std::endl;

  return 0;
}
