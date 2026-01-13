#!/usr/bin/env bash

usage () {
	echo "usage: new_test.sh name"
	echo "!!NOT!! new_test.sh test_name.cpp"
}

if [[ ! -e cpp_tests || ! -d cpp_tests ]]; then
	echo "cpp_tests dir not found!"
	exit 1
fi

if [ $# -eq 0 ]; then
	usage
	exit 1
fi

if [[ $(echo $1 | grep .cpp) || $(echo $1 | grep test)  ]]; then
	usage
	exit 1
fi

# test_fname=$(echo $1.cpp)
test_fname="cpp_tests/test_$1.cpp"
if [ -e $test_fname ]; then
	echo ">> errror: test file already exists!"
	exit 1
fi
cp ./test_template.cpp $test_fname
sed -i "s/template/$1/" $test_fname
