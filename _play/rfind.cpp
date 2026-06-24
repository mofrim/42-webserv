#include <iostream>
#include <string>

int main(int ac, char **av)
{
  (void)ac, (void)av;

  std::string s = "hallo welt miep moep!";

  std::cout << "s.rfind(welt) + std::string(welt).length(): "
            << s.rfind("welt") + std::string("welt").length() << std::endl;

  return 0;
}
