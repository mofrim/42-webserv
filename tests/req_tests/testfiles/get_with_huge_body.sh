#!/usr/bin/env bash

# Test send a just one line too large header to the server.

set -euo pipefail

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

	echo "doing it with '$1'"
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
sendHdrField "Host: moep" 3
finishReq 3
cat < ./hugefile >&3

RESPONSE="$(head -n 115 <&3)"
echo "${RESPONSE[@]}"

exec 3<&-

if [ -z "$RESPONSE" ]; then
	exit 1;
fi

