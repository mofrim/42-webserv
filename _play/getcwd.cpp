#include <cstring>
#include <iostream>

int main(int ac, char **av, char **env)
{
  (void)ac, (void)av;

  char **p = env;
  while (*p) {
    if (strlen(*p) >= 3 && !strncmp(*p, "PWD", 3))
      std::cout << "PWD: " << *p << std::endl;
    p++;
  }

  return 0;
}
