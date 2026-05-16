#!/usr/bin/env bash

# testing against allowed methods cfg

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
	$webserv $cfgDir/allowedMethods.wsrv > /dev/null &
	sleep 0.1s
fi

exec 3<>/dev/tcp/"$hostname"/"$port"

# -----------=[ test for GET on del route. should fail with 403 ]=----------- #

sendHdrField "GET /del HTTP/1.1" 3
sendHdrField "Host: miep" 3
finishReq 3

RESPONSE="$(timeout 0.1s cat <&3 2>/dev/null | grep 403)"
echo "Response:"
echo "---------"
echo "${RESPONSE[@]}"

if [ -z "$RESPONSE" ]; then
	exit 1;
fi

exec 3<&-

sleep 0.1s

# ---------------=[ test DELETE on del route. should be 200. ]=--------------- #

exec 3<>/dev/tcp/"$hostname"/"$port"

sendHdrField "DELETE /del HTTP/1.1" 3
sendHdrField "Host: miep" 3
finishReq 3

RESPONSE="$(timeout 0.1s cat <&3 2>/dev/null | grep 404)"
echo "Response:"
echo "---------"
echo "${RESPONSE[@]}"

if [ -z "$RESPONSE" ]; then
	exit 1;
fi

exec 3<&-

sleep 0.1s

# ----------------=[ test DELETE and POST on postdel route ]=---------------- #

exec 3<>/dev/tcp/"$hostname"/"$port"

sendHdrField "DELETE /postdel HTTP/1.1" 3
sendHdrField "Host: miep" 3
finishReq 3

RESPONSE="$(timeout 0.1s cat <&3 2>/dev/null | grep 404)"
echo "Response:"
echo "---------"
echo "${RESPONSE[@]}"

if [ -z "$RESPONSE" ]; then
	exit 1;
fi

sleep 0.1s

exec 3<>/dev/tcp/"$hostname"/"$port"

sendHdrField "POST /postdel HTTP/1.1" 3
sendHdrField "Host: miep" 3
sendHdrField "Content-Length: 16" 3
finishReq 3
echo -n "this is the body" >&3

RESPONSE="$(timeout 0.2s cat <&3 2>/dev/null | grep 500)"
echo "Response:"
echo "---------"
echo "${RESPONSE[@]}"

if [ -z "$RESPONSE" ]; then
	exit 1;
fi

exec 3<&-

# SIGINT kill webserv
if [ $# -eq 2 ]; then
	pkill -INT webserv
fi


