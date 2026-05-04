#!/usr/bin/env bash

# template

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

sendHdrField "GET / HTTP/1.1" 3
sendHdrField "Host: miep" 3
sendHdrField "Connection: klshjdasjdalsjd" 3
finishReq 3
RESPONSE="$(timeout 5s cat <&3)"
echo "Response:"
echo
echo "${RESPONSE[@]}"

exec 3<&-

if [ -z "$RESPONSE" ]; then
	exit 1;
fi

