#include <iostream>
#include <string>

int main(int ac, char **av)
{
  (void)ac, (void)av;

  char raw[] = {'a', 'b', 'c', 0, 'd'};

  std::string str(raw);

  std::cout << "str: " << str << std::endl;
  std::cout << "str.size() = " << str.size() << std::endl;

  std::string str2;
  str2.append(raw, 5);

  std::cout << "str2: " << str2 << std::endl;
  std::cout << "str2.size() = " << str2.size() << std::endl;

  std::string str3 = str2.substr(0);

  std::cout << "str3: " << str3 << std::endl;
  std::cout << "str3.size() = " << str3.size() << std::endl;

  std::string str_raw;
  str_raw.assign(raw, 5);

  std::cout << "str_raw: " << str_raw << std::endl;
  std::cout << "str_raw.c_str(): " << str_raw.c_str() << std::endl;
  std::cout << "str_raw.data(): " << str_raw.data() << std::endl;
  std::cout << "str_raw.size() = " << str_raw.size() << std::endl;

  const char *p = str_raw.data();

  for (size_t i = 0; i < str_raw.size(); i++)
    std::cout << "p: " << *p++ << std::endl;

  const char *cstr = str_raw.c_str();
  const char *data = str_raw.data();

  if (cstr == data)
    std::cout << "You see? c_str and data are the same!" << std::endl;

  return 0;
}
