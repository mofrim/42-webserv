#!/usr/bin/env bash

CRLF=$'\r\n'

function sendHdrField() {
	echo -en "$1$CRLF"
}

function finishReq() {
	echo -en "$CRLF"
}

IFS= read -r -d '' name

msg="Hello <$name> from mofrim's WebServ!\n"
len="${#msg}"

sendHdrField "Content-Type: text/html"
sendHdrField "Content-Length: $len"
finishReq
echo -en "$msg"
