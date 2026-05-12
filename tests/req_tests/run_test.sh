#!/usr/bin/env bash

allgood=1

# set port and addr here
addr="localhost"
port="1111"

# i need my utils!
if [ ! -e ./_test_utils.sh ]; then
	echo -e "\e[31m./_test_utils.sh not found!\e[0m"
	exit 1
else
	source ./_test_utils.sh
fi

# check if webserv is running, otherwise running these tests does not make much
# sense
if [ ! pidof webserv &> /dev/null ] && [ ! pidof valgrind &>/dev/null ]; then
	echo -e "\n\e[31meeehm... webserv must be running for this!\e[0m\n"
	exit 1
fi

# for skipping tests add their basename here. this is for especially for those
# who would take too long
skip_tests=(
	"get_with_huge_body.sh"
)

cd testfiles

echo -e "\n-----=[ \e[36mRunning m0fr1m's webserv tests\e[0m ]=-----\n"

for i in `ls *.sh`;
do
	if ! arrContains "$i" "${skip_tests[@]}"; then
		./$i $addr $port > /dev/null
		if [ $? -ne 0 ]; then
			echo -e "  \e[31mKO\e[0m: test '$i' failed!"
			allgood=0
		else
			echo -e "  \e[32mOK\e[0m: test '$i' passed!"
		fi
	fi
done

if [ $allgood -eq 1 ]; then 
	echo -e "\n-----=[ \e[1;32mOK\e[36m, all tests passed \o/\e[0m ]=-----\n"
else
	echo -e "\n-----=[ \e[1;31mKO\e[36m, some tests failed ;(\e[0m ]=-----\n"
fi
