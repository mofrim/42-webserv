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
	$webserv $cfgDir/cgi.wsrv > /dev/null &
	sleep 0.1s
fi

exec 3<>/dev/tcp/"$hostname"/"$port"

# ------------------------=[ test logic starts here ]=------------------------ #

# simulate a failure during Response::cgiWrite

sendHdrField "POST /long_sleep.sh HTTP/1.1" 3
sendHdrField "Host: miep" 3
sendHdrField "Content-Length: 5" 3
finishReq 3
echo -n "hallo" >&3

RESPONSE="$(timeout 0.2s cat <&3)"
echo "Response:"
echo "---------"
echo "${RESPONSE[@]}"

sleep 0.5s

exec 3<&-

tmpfile="$(mktemp)"
findSleep="$(ps ax > $tmpfile && cat $tmpfile | grep "sleep 123s" && rm $tmpfile)"
echo "findSleep = $findSleep";

# SIGINT kill webserv
if [ $# -eq 2 ]; then
	pkill -INT webserv
fi

if [ -n "$findSleep" ]; then
	exit 1;
fi

