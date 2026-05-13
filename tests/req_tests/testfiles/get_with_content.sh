#!/usr/bin/env bash

# GET with Content-Length will simply be ignored and webserved


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

sendHdrField "GET / HTTP/1.1" 3
sendHdrField "Host: miep" 3
sendHdrField "Content-Length: 100" 3
finishReq 3
RESPONSE="$(timeout 0.1s cat <&3 | grep 200)"
exec 3<&-

# SIGINT kill webserv
if [ $# -eq 2 ]; then
	pkill -INT webserv
fi

if [ -z "$RESPONSE" ]; then
	exit 1;
fi

