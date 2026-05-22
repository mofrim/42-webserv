#!/usr/bin/env bash

CRLF=$'\r\n'

function sendHdrField() {
	echo -en "$1$CRLF"
}

function finishReq() {
	echo -en "$CRLF"
}

msg="$(head -n 2)"
exit 1
