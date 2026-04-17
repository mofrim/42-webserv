#!/usr/bin/env bash

usage () {
	echo "usage: new_test.sh name"
	echo "!!NOT!! new_test.sh test_name.cpp"
}

TEST_FILES_DIR=tests
TEMPLATE_DIR=templates

if [[ ! -e $TEST_FILES_DIR || ! -d $TEST_FILES_DIR ]]; then
	echo "$TEST_FILES_DIR dir not found!"
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
test_fname="$TEST_FILES_DIR/test_$1.cpp"
if [ -e $test_fname ]; then
	echo ">>> errror: test file already exists!"
	exit 1
fi
cp $TEMPLATE_DIR/test-template.cpp $test_fname
sed -i "s/template/$1/" $test_fname

echo ">>> $test_fname is waiting for you"
