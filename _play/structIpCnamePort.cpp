
#include <iostream>
#include <set>
#include <stdint.h>
#include <string>

typedef std::string str;
typedef uint16_t    u16;

typedef struct {
    str                             cname;
    str                             ip;
    std::set< std::pair<int, u16> > fdPort;
} t_vsrvInterface;

bool operator==(const t_vsrvInterface& i1, const t_vsrvInterface& i2)
{
  if (i1.ip == i2.ip && i1.cname == i2.cname)
    return true;
  return false;
}

// less than op needed for std::set
bool operator<(const t_vsrvInterface& i1, const t_vsrvInterface& i2)
{
  if (i1.cname < i2.cname)
    return true;
  return false;
}

int main(int ac, char **av)
{
  (void)ac, (void)av;

  t_vsrvInterface m;

  m.ip    = "1.1.1.1";
  m.cname = "hunpf";
  m.fdPort.insert(std::make_pair(1, 2));
  m.fdPort.insert(std::make_pair(3, 4));

  std::cout << "m.ip: " << m.ip << std::endl;
  std::cout << "m.cname: " << m.cname << std::endl;
  std::cout << "m.ports: " << std::endl;

  for (std::set< std::pair<int, u16> >::iterator it = m.fdPort.begin();
      it != m.fdPort.end();
      it++)
    std::cout << it->first << ", " << it->second << std::endl;

  t_vsrvInterface m1;

  m1.ip    = "1.1.1.1";
  m1.cname = "hunpf";
  m.fdPort.insert(std::make_pair(100, 200));
  m.fdPort.insert(std::make_pair(300, 400));

  if (m == m1)
    std::cout << "they are the same!" << std::endl;
  else
    std::cout << "they are NOT the same!" << std::endl;

  std::cout << "sizeof(t_vsrvInterface)" << sizeof(t_vsrvInterface)
            << std::endl;

  std::set<t_vsrvInterface> s;
  s.insert(m);
  std::set<t_vsrvInterface>::iterator it = s.begin();

  it->fdPort.insert(std::make_pair(123, 123));

  // if (s.find(m1) != s.end()) {
  //   std::cout << "found m1 in s! only inserting ports" << std::endl;
  //   s.find(m1)->fdPort.insert(m1.fdPort.begin(), m1.fdPort.end());
  // }
  // else
  //   std::cout << "did not find m1 in s :(" << std::endl;

  return 0;
}
