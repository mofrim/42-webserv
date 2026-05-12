#!/usr/bin/env bash

# GET with Content-Length will simply be ignored and webserved


set -u

if [ ! -e ../_test_utils.sh ]; then
	echo "test-utils not found!"
	exit 1
else
	source ../_test_utils.sh
fi

exec 3<>/dev/tcp/"$1"/"$2"

sendHdrField "GET / HTTP/1.1" 3
sendHdrField "Host: miep" 3
sendHdrField "Content-Length: 100" 3
finishReq 3
RESPONSE="$(timeout 0.1s cat <&3 | grep 200)"
exec 3<&-

if [ -z "$RESPONSE" ]; then
	exit 1;
fi

