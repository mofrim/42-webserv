#!/usr/bin/env bash

CRLF=$'\r\n'

function sendHdrField() {
	echo -en "$1$CRLF"
}

function finishReq() {
	echo -en "$CRLF"
}

msg="$(head -n 2)"
echo "msg: $msg" 1>&2
exit 1


# sendHdrField "Content-Type: text/plain"
# sendHdrField "Content-Length: $len"
# finishReq
# echo -en "$msg"
