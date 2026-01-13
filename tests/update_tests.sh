#!/usr/bin/env bash

# this expands a glob of nothing into empty string.
shopt -s nullglob

if [[ ! -e cpp_tests || ! -d cpp_tests ]]; then
	echo "cpp_tests dir not found!"
	exit 1
fi

wsrv_makefile="../Makefile"
mainfile="test-main.cpp"
testhdr="tests.hpp"
tf_func=""
tf_fun_arr=()
tmpf="$(mktemp)"

for tf in cpp_tests/*; do
	tf_func="$(basename -s .cpp $tf)"
	echo -e "\t\t$(basename $tf) \\" >> $tmpf
	echo "test found: $tf_func"
	tf_fun_arr+=("$tf_func")
done

# populating test_main.cpp:
echo "#include \"test-utils.hpp\"" > $mainfile
if [ $? -ne 0 ]; then
	echo "error writing to test_main.cpp"
	exit 1
fi

echo "#include \"tests.hpp\"" >> $mainfile
echo "int main()" >> $mainfile

echo "{" >> $mainfile
for func in ${tf_fun_arr[@]}; do
	echo -e "\tprint_test_file_header(\"$func\");" >> $mainfile
	echo -e "\t$func();" >> $mainfile
done
echo "}" >> $mainfile

# populating tests.hpp:

echo "#ifndef TESTS_HPP" > $testhdr
if [ $? -ne 0 ]; then
	echo "error writing to tests.hpp"
	exit 1
fi
echo "#define TESTS_HPP" >> $testhdr

for func in ${tf_fun_arr[@]}; do
	echo -e "void $func();" >> $testhdr
done
echo "#endif" >> $testhdr

# testing makefile

cp Makefile.base Makefile
sed -i "/test-utils\.cpp/r $tmpf" Makefile
sed -i 's/test-utils\.cpp/test-utils\.cpp \\/g' Makefile
last_file="$(cat $tmpf | tail -n 1 | sed 's/\\//')"
last_file_double_bs="$(cat $tmpf | tail -n 1 | sed 's/\\/\\\\/')"
sed -i "s/$last_file_double_bs/$last_file/" Makefile
rm -f $tmpf

# fill in all srcs and headers we currently have available in our proj

cpp_files=$(ls -x -w 0 ../src)
wsrv_srcs=$(echo "WSRV_SRCS = $cpp_files" | sed 's/main.cpp//')
hpp_files=$(ls -x -w 0 ../inc)
wsrv_hdrs=$(echo "WSRV_HDRS = $cpp_files")
sed -i "s/WSRV_SRCS_HERE/$wsrv_srcs/" Makefile
sed -i "s/WSRV_HDRS_HERE/$wsrv_hdrs/" Makefile



