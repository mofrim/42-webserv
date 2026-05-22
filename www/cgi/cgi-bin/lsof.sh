#!/usr/bin/env bash

CRLF=$'\r\n'

function sendHdrField() {
	echo -en "$1$CRLF"
}

function finishReq() {
	echo -en "$CRLF"
}


msg="$(lsof -c webserv)"

if [ -z "$msg" ]; then
	msg="$(lsof -p $(pidof valgrind))"
fi

if [ -z "$msg" ]; then
	exit 1
fi

len="${#msg}"
# echo "msglen = $len" 1>&2
# echo "pipe-max-size: $(cat /proc/sys/fs/pipe-max-size)" 1>&2

sendHdrField "Content-Type: text/plain"
sendHdrField "Content-Length: $len"
finishReq
echo -n "$msg"

# dd if=/dev/random bs=500000001 count=1

# echo "$msg" > lsof-out.dat
# echo "$msg"

