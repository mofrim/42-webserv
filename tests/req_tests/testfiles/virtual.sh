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

sendHdrField "GET / HTTP/1.1" 3
sendHdrField "Host: v1" 3
finishReq 3

RESPONSE="$(timeout 0.1s cat <&3 | grep 'Welcome to V1')"
echo "Response:"
echo "---------"
echo "${RESPONSE[@]}"

if [ -z "$RESPONSE" ]; then
	# SIGINT kill webserv
	if [ $# -eq 2 ]; then
		pkill -INT webserv
	fi
	exit 1;
fi

# -------------------------=[ other virtual server ]=------------------------- #

sendHdrField "GET / HTTP/1.1" 3
sendHdrField "Host: v2" 3
finishReq 3

RESPONSE="$(timeout 0.1s cat <&3 | grep 'Welcome to V2')"
echo "Response:"
echo "---------"
echo "${RESPONSE[@]}"

if [ -z "$RESPONSE" ]; then
	# SIGINT kill webserv
	if [ $# -eq 2 ]; then
		pkill -INT webserv
	fi
	exit 1;
fi

# -----------------=[ not a virtual server but wrong iface ]=----------------- #

sendHdrField "GET / HTTP/1.1" 3
sendHdrField "Host: vNot" 3
finishReq 3

RESPONSE="$(timeout 0.1s cat <&3 | grep 'Welcome to V1')"
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

