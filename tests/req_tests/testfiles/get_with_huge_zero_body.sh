#!/usr/bin/env bash

# sending huge body with GET. get 200 at first but 400 and client close later
# create hugefile using: `dd if=/dev/random of=./hugefile bs=1024 count=100000 `
# this will be 100MiB

if [ $# -lt 2 ]; then
	echo "need at least 2 args: [-n] addr port"
	exit 1
fi

if [[ $# -eq 3 && "$1" != "-n" ]]; then
	echo "usage with 3 args: $0 -n addr port"
	exit 1
fi

if [ ! -e ../_test_utils.sh ]; then

	echo "test-utils not found!"
	exit 1
else
	source ../_test_utils.sh
fi

set -u

# change interface here if required
if [ $# -eq 3 ]; then
	hostname="$2"
	port="$3"
else
	hostname="$1"
	port="$2"
	$webserv $cfgDir/simplest.wsrv > /dev/null &
	sleep 0.1s
fi

exec 3<>/dev/tcp/"$hostname"/"$port"

# ------------------------=[ test logic starts here ]=------------------------ #

sendHdrField "GET / HTTP/1.1" 3
sendHdrField "Host: moep" 3
finishReq 3

cat < ./zero >&3

RESPONSE="$(timeout 0.3s cat <&3 2>/dev/null | grep -E '200|400')"
echo "Response:"
echo "---------"
echo "${RESPONSE[@]}"

exec 3<&-

# SIGINT kill webserv
if [ $# -eq 2 ]; then
	pkill -INT webserv
fi

if [[ -z "$(echo ${RESPONSE[@]} | grep -E '200|400')" ]]; then
	exit 1;
fi

