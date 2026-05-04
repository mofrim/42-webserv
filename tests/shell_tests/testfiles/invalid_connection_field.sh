#!/usr/bin/env bash

# Test sending a non-sense Connection hdr field. Nginx handles this very
# tolerantly by ignoring this.

set -u

CRLF="\r\n"

if [ $# -ne 2 ]; then
	echo "need 2 args: addr port"
	exit 1
fi

function sendHdrField() {
	if [ $# -ne 2 ]; then
		echo "need 2 args: msg fd"
		exit 1
	fi
	echo -en "$1$CRLF" >&$2
}

function finishReq() {
	if [ $# -ne 1 ]; then
		echo "need 2 args: fd"
		exit 1
	fi

	echo -en "$CRLF" >&$1
}

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

