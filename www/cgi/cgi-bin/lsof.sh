#!/usr/bin/env bash

set -e

CRLF=$'\r\n'

function sendHdrField() {
	echo -en "$1$CRLF"
}

function finishReq() {
	echo -en "$CRLF"
}

# msg="$(lsof)"
# len="${#msg}"
# echo "msglen = $len" 1>&2
# echo "pipe-max-size: $(cat /proc/sys/fs/pipe-max-size)" 1>&2

sendHdrField "Content-Type: text/plain"
sendHdrField "Content-Length: $len"
finishReq

dd if=/dev/random bs=500000001 count=1

# echo "$msg" > lsof-out.dat
# echo "$msg"

