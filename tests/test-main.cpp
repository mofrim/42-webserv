#include "test-utils.hpp"
#include "tests.hpp"
int main()
{
	print_test_file_header("test_example1");
	test_example1();
	print_test_file_header("test_example2");
	test_example2();
}
