#!/usr/bin/env bash

# empty req

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

finishReq 3

RESPONSE="$(timeout 0.1s cat <&3)"
echo "Response:"
echo "---------"
echo "${RESPONSE[@]}"

exec 3<&-

# SIGINT kill webserv
kill -INT %1

# fail if we got a response
if [ -n "$RESPONSE" ]; then
	exit 1;
fi

