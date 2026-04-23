#include "test_hdr.hpp"
#include "test_utils.hpp"
#include "tests.hpp"
bool g_GlobalResult = OK;
int  main()
{
  test_server_init();
  nl();
  test_VServerCfg();
  nl();
  test_socket();
  nl();
  test_HttpStatus();
  nl();
  test_Routes();
  nl();
  print_final_result();
  if (g_GlobalResult == OK)
    print_test_result(OK, "All tests passed 8)");
  else
    print_test_result(KO, "At least one test failed :/");
  return 0;
}
