#include <ctime>
#include <iostream>
#include <unistd.h>

int main(int ac, char **av)
{
  (void)ac, (void)av;

  time_t t0 = time(NULL);

  sleep(3);

  time_t t1 = time(NULL);

  std::cout << "time passed: " << difftime(t1, t0) << std::endl;

  return 0;
}
