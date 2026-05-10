#!/usr/bin/env bash

# ---------------------------------------------------------------------------- #
# -------------------=[ m0fr1m's awesome update_tests.sh ]=------------------- #
# ---------------------------------------------------------------------------- #

# this expands a glob of nothing into empty string.
shopt -s nullglob
set -euo pipefail

# -------------------------=[ configurable things ]=------------------------- #

TEST_FILES_DIR=tests
PROJ_MAKEFILE="../../Makefile"
TEST_MAIN="test_main.cpp"
TEST_HDR="tests.hpp"
PROJ_DIR="../.."
PROJ_SRCS_DIR=../../src
PROJ_HDRS_DIR=../../inc
PROJ_OBJ_DIR=../../obj
TEMPLATE_DIR=templates

# -----------------------------=[ helper funcs ]=----------------------------- #

sanitizePath() {
	echo $1 | sed -e 's/\./\\\./g' -e 's/\//\\\//g'
}

# -------------------------------=[ the rest ]=------------------------------- #

if [[ ! -e $TEST_FILES_DIR || ! -d $TEST_FILES_DIR ]]; then
	echo "TEST_FILES_DIR \"$TEST_FILES_DIR\" not found!"
	exit 1
fi

tf_func=""
tf_fun_arr=()
tmpf="$(mktemp)"

# use `ls -cr` to list by ctime and show the newest one last. this will lead to
# the newest test being the last one run \o/
for tf in `ls -cr $TEST_FILES_DIR/*.cpp`; do
	tf_func="$(basename -s .cpp $tf)"
	echo -e "\t\t$(basename $tf) \\" >> $tmpf
	echo "test found: $tf_func"
	tf_fun_arr+=("$tf_func")
done

# populating test_main.cpp:

echo "#include \"tests.hpp\"" > $TEST_MAIN
if [ $? -ne 0 ]; then
	echo "error writing to test_main.cpp"
	exit 1
fi
cat <<EOF >> $TEST_MAIN
#include "test-utils.hpp"
#include "test_hdr.hpp"
bool g_GlobalResult = OK;
int main()
{
  g_WsrvTesting = true;
EOF
for func in ${tf_fun_arr[@]}; do
	echo -e "  $func();" >> $TEST_MAIN
	echo -e "  nl();" >> $TEST_MAIN
done 
cat <<EOF >> $TEST_MAIN
  print_final_result_header();
  if (g_GlobalResult == OK)
    print_final_result(OK, "All tests passed 8)");
  else
    print_final_result(KO, "At least one test failed :/");
  return 0;
}
EOF

# populating tests.hpp:

echo "#pragma once" > $TEST_HDR
if [ $? -ne 0 ]; then
	echo "error writing to tests.hpp"
	exit 1
fi
for func in ${tf_fun_arr[@]}; do
	echo -e "void $func();" >> $TEST_HDR
done

# populating makefile

cp $TEMPLATE_DIR/Makefile.base Makefile
sed -i "/test-utils\.cpp/r $tmpf" Makefile
sed -i 's/test-utils\.cpp/test-utils\.cpp \\/g' Makefile
last_file="$(cat $tmpf | tail -n 1 | sed 's/\\//')"
last_file_double_bs="$(cat $tmpf | tail -n 1 | sed 's/\\/\\\\/')"
if [ -n "$last_file" ] && [ -n "$last_file_double_bs" ]; then
	sed -i "s/$last_file_double_bs/$last_file/" Makefile
fi
rm -f $tmpf

# fill in all srcs and headers we currently have available in our proj

cpp_files=$(ls -x -w 0 ${PROJ_SRCS_DIR})
proj_srcs=$(echo "PROJ_SRCS = $cpp_files" | sed 's/main.cpp//')
hpp_files=$(ls -x -w 0 ${PROJ_HDRS_DIR})
proj_hdrs=$(echo "PROJ_HDRS = $hpp_files")
sed -i "s/PROJ_SRCS_HERE/$proj_srcs/" Makefile
sed -i "s/PROJ_HDRS_HERE/$proj_hdrs/" Makefile
sed -i "s/PROJ_HDRS_DIR_HERE/$(sanitizePath $PROJ_HDRS_DIR)/" Makefile
sed -i "s/PROJ_SRCS_DIR_HERE/$(sanitizePath $PROJ_SRCS_DIR)/" Makefile
sed -i "s/PROJ_OBJ_DIR_HERE/$(sanitizePath $PROJ_OBJ_DIR)/" Makefile
sed -i "s/PROJ_DIR_HERE/$(sanitizePath $PROJ_DIR)/" Makefile

# populate proj_hdrs.hpp
cp $TEMPLATE_DIR/test_hdr-template.hpp test_hdr.hpp
ls -1 $PROJ_HDRS_DIR | sed 's/.*/#include "&"/g' >> test_hdr.hpp
