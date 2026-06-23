#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>

int main()
{
  sleep(2);

  std::string s;

  for (size_t i = 0; i < 1000000; i++) {
    s.append(
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
        "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
    usleep(1);
  }
  sleep(5);
  std::cout << "clearing string..." << std::endl;

  std::string().swap(s);

  std::string s2 = "hellohello";

  std::cout << "done! sleeping another round..." << std::endl;
  sleep(5);
  return 0;
}

// int main()
// {
//   std::cout << "initial sleep(2)..." << std::endl;
//   sleep(2);
//
//   std::cout << "creating 100MB data..." << std::endl;
//   char *s = new char[100000000];
//   for (size_t i = 0; i <= 1000000; i++) {
//     memcpy(s + i * 100,
//         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
//         "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
//         100);
//     usleep(1);
//   }
//   sleep(2);
//   std::cout << "clearing string..." << std::endl;
//   delete[] s;
//   std::cout << "done! sleeping another round..." << std::endl;
//   sleep(5);
//   return 0;
// }
