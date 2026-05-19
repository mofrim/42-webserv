#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

int main(int ac, char **av)
{
  (void)ac, (void)av;

  std::vector<char> sv;
  std::string       s("moep");
  sv.assign(s.begin(), s.end());

  write(STDOUT_FILENO, &sv[0], sv.size());

  return 0;
}
