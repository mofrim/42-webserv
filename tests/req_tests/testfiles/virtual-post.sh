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
	$webserv $cfgDir/virtual.wsrv > /dev/null &
	sleep 0.1s
fi

exec 3<>/dev/tcp/"$hostname"/"$port"

# ------------------------=[ test logic starts here ]=------------------------ #

sendHdrField "POST /post HTTP/1.1" 3
sendHdrField "Host: v1" 3
sendHdrField "Content-Length: 4" 3
finishReq 3
echo "bla" >&3

RESPONSE="$(timeout 0.1s cat <&3)"
echo "Response:"
echo "---------"
echo "${RESPONSE[@]}"

file=$(echo "${RESPONSE[@]}" | grep "Location" | tr -s ' \t' | cut -d ':' -f 2 | tr -d ' \r\n')

RESPONSE=$(echo "${RESPONSE[@]}" | grep 201)

if [ -z "$(echo "${RESPONSE[@]}" | grep 201)" ]; then
	exec 3<&-
	if [ $# -eq 2 ]; then
		pkill -INT webserv
	fi
	exit 1;
fi

reqline="DELETE $file HTTP/1.1"
sendHdrField "$reqline" 3
sendHdrField "Host: v1" 3
finishReq 3

RESPONSE="$(timeout 0.1s cat <&3 | grep 204)"
echo "Response:"
echo "---------"
echo "${RESPONSE[@]}"

exec 3<&-

# SIGINT kill webserv
if [ $# -eq 2 ]; then
	pkill -INT webserv
fi

if [ -z "$RESPONSE" ]; then
	exit 1;
fi

