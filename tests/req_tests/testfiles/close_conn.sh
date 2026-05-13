#!/usr/bin/env bash

# testing Connection: close and keep-alive. If `close` is given the `timeoute
# cat` should return with exit status 0 because the `cat` will not need to be
# timed out. On the other hand, if `keep-alive` is given, there will exit status
# 124.

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
sendHdrField "Connection: keep-alive" 3
finishReq 3

RESPONSE="$(timeout 0.5s cat <&3)"

if [[ $? -ne 124 || -z "$RESPONSE" ]]; then
	exit 1;
fi

exec 3<>/dev/tcp/"$1"/"$2"

sendHdrField "GET / HTTP/1.1" 3
sendHdrField "Host: miep" 3
sendHdrField "Connection: close" 3
finishReq 3

RESPONSE="$(cat <&3)"

exec 3<&-

# SIGINT kill webserv
kill -INT %1

if [[ $? -ne 0 || -z "$RESPONSE" ]]; then
	exit 1;
fi

