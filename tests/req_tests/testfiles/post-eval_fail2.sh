#!/usr/bin/env bash

# post exceeds maxBodySize for the route -> should 413

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
	$webserv $cfgDir/bigFileUpload.wsrv > /dev/null &
	sleep 0.1s
fi

exec 3<>/dev/tcp/"$hostname"/"$port"

# ------------------------=[ test logic starts here ]=------------------------ #

# this one should fail with a 408 even though the Content-Length is bigger than
# maxBodySize but it is too big for the body being sent! i.e. the server has to
# wait for more data to come for draining, but this never happens
# maxBodySize for this endpoint is 10

sendHdrField "POST /small HTTP/1.1" 3
sendHdrField "Host: miep" 3
sendHdrField "Content-Length: 1000" 3
finishReq 3
echo -n "BODY IS HERE write something shorter or longer than body limit" >&3

tmpfile="$(mktemp)"
timeout 6s cat <&3 > $tmpfile &
echo sleeping
sleep 4 && curl -s localhost:1234 &>/dev/null
RESPONSE="$(cat $tmpfile)"
rm -f $tmpfile

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

