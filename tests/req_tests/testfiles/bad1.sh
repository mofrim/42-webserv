#!/usr/bin/env bash

# malformed hdr field, 400 page expected

if [ $# -ne 2 ]; then
	echo "need 2 args: addr port"
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
hostname="$1"
port="$2"

$webserv $cfgDir/simplest.wsrv > /dev/null &

sleep 0.1s

exec 3<>/dev/tcp/"$hostname"/"$port"

# ------------------------=[ test logic starts here ]=------------------------ #
#
sendHdrField "GET / HTTP/1.1" 3
sendHdrField "Host miep" 3
finishReq 3

RESPONSE="$(timeout 0.1s cat <&3 | grep 400)"
echo "Response:"
echo
echo "${RESPONSE[@]}"

exec 3<&-

# SIGINT kill webserv
kill -INT %1

if [ -z "$RESPONSE" ]; then
	exit 1;
fi

