#include "tests.hpp"
#include "test_utils.hpp"
#include "test_hdr.hpp"
bool g_GlobalResult = OK;
int main()
{
  test_server_init();
  nl();
  test_socket();
  nl();
  test_VServerCfg();
  nl();
  print_final_result();
  if (g_GlobalResult == OK)
    print_test_result(OK, "All tests passed 8)");
  else
    print_test_result(KO, "At least one test failed :/");
  return 0;
}
