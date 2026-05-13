#!/usr/bin/env bash

# Send req to server which is never terminated. Make parallel request to trigger
# timeout.

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

CRLF="\r\n"

exec 3<>/dev/tcp/"$1"/"$2"

sendHdrField "GET / HTTP/1.1" 3
sendHdrField "host: moep" 3
tmpfile=$(mktemp)
timeout 7s cat <&3 > $tmpfile &
sleep 6 && curl "$1:$2" &> /dev/null

exec 3<&-

cat $tmpfile

if [ -z "$(cat $tmpfile | grep 408)" ]; then
	rm $tmpfile
	exit 1;
fi

rm -f $tmpfile


