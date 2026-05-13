#!/usr/bin/env bash

# hit redir target

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

exec 3<>/dev/tcp/"$1"/"$2"

sendHdrField "GET /redir HTTP/1.1" 3
sendHdrField "Host: miep" 3
finishReq 3

RESPONSE="$(timeout 0.1s cat <&3)"
echo "Response:"
echo
echo "${RESPONSE[@]}"

exec 3<&-

# fail if we got a response
if [ -n "$RESPONSE" ]; then
	exit 1;
fi

