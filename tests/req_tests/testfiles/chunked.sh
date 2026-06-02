#!/usr/bin/env bash

# template

# ---------------------------=[ test boilerplate ]=--------------------------- #

if [ $# -lt 2 ]; then
	echo "need at least 2 args: [-n] addr port"
	exit 1
fi

if [[ $# -eq 3 && "$1" != "-n" ]]; then
	echo "usage with 3 args: $0 -n addr port"
	exit 1
fi

if [ ! -e ../_test_utils.sh ]; then
	echo "test-utils not found!"
	exit 1
else
	source ../_test_utils.sh
fi

set -u

if [ $# -eq 3 ]; then
	hostname="$2"
	port="$3"
else
	hostname="$1"
	port="$2"
	$webserv $cfgDir/post.wsrv > /dev/null &
	sleep 0.1s
fi

exec 3<>/dev/tcp/"$hostname"/"$port"

# ------------------------=[ test logic starts here ]=------------------------ #

sendHdrField "POST / HTTP/1.1" 3
sendHdrField "Host: miep" 3
sendHdrField "Transfer-Encoding: chunked ; gzip" 3
finishReq 3
sendHdrField "1" 3
sendHdrField "a" 3
sendHdrField "1" 3
sendHdrField "b" 3
sendHdrField "1" 3
sendHdrField "c" 3
sendHdrField "0" 3


RESPONSE="$(timeout 0.1s cat <&3)"
echo "Response:"
echo "---------"
echo "${RESPONSE[@]}"

file="../../_www$(echo "${RESPONSE[@]}" | grep Location | cut -d ' ' -f 2 | tr -d '\r\n')"
echo "file: '$file'"

exec 3<&-

# SIGINT kill webserv
if [ $# -eq 2 ]; then
	pkill -INT webserv
fi

if [[ ! -e $file && "$(cat $file)" != "abc" ]]; then
	echo "Uploaded file not found or content false!"
	exit 1
fi

rm -f "$file"

if [ -z "$RESPONSE" ]; then
	exit 1;
fi

