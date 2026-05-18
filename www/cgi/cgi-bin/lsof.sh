#!/usr/bin/env bash

CRLF=$'\r\n'

function sendHdrField() {
	echo -en "$1$CRLF"
}

function finishReq() {
	echo -en "$CRLF"
}

msg="$(lsof -c webserv)"
len="${#msg}"
echo "len = $len" 1>&2


sendHdrField "Content-Type: text/plain"
sendHdrField "Content-Length: $len"
finishReq

# sleep 10s

echo -en "$msg"
