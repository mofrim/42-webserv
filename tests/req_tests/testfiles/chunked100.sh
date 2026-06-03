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
	$webserv $cfgDir/42tester.wsrv > /dev/null &
	sleep 0.1s
fi

exec 3<>/dev/tcp/"$hostname"/"$port"

# ------------------------=[ test logic starts here ]=------------------------ #

sendHdrField "POST /post_body HTTP/1.1" 3
sendHdrField "Host: miep" 3
sendHdrField "Transfer-Encoding: chunked" 3
finishReq 3
echo -en "64$CRLF" >&3
echo -en "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa" >&3
echo -en "$CRLF" >&3
echo -en "0$CRLF$CRLF" >&3

RESPONSE="$(timeout 0.2s cat <&3 | grep 201)"
echo "Response:"
echo "---------"
echo "${RESPONSE[@]}"


# tmpfile=$(mktemp)
# timeout 6s cat <&3 > $tmpfile &
# sleep 6 && curl "$hostname:$port" &> /dev/null

exec 3<&-

# SIGINT kill webserv
if [ $# -eq 2 ]; then
	pkill -INT webserv
fi

if [ -z "$RESPONSE" ]; then
	exit 1;
fi
