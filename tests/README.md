# Testing Howto

1) create a new test `cpp`-file in the cpp_tests dir.
2) in that file there _must_ be a function with the same name as the filename.
   example:

    filename: `test_server_init.cpp`
    function in file: `void test_server_init() {...}`

  also the filename must be prefixed with `test`. so `blalba_test.cpp` is not a
  valid testfile-name! (this is only conventional)
3) run `./update_tests.sh` script in order to generate a `test-main.cpp` which
   will call all the test functions. the `Makefile` will be generated from
   `Makefile.base` with all the test cpp files inserted. A header file
   `tests.hpp` will also be generated.
4) run `make` and exec `./webserv_tests`

## Notes

- the whole thing can easily be run from repo-root with `make tests`
- cleanup can be done by calling `make clean` or `make tests-clean` from
  repo-root
- **TODO**: automatically populate Makefile with all cpp and hpp files from `src`
  and `inc` dirs.
