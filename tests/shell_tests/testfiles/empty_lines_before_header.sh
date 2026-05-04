#!/usr/bin/env bash

# RFC demands we SHOULD support at least one empty line before hdr starts. so we
# do it!

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

sendHdrField "" 3
sendHdrField "GET / HTTP/1.1" 3
sendHdrField "Host: miep" 3
finishReq 3
RESPONSE="$(timeout 1s cat <&3 | grep 200)"
exec 3<&-

if [ -z "$RESPONSE" ]; then
	exit 1;
fi

