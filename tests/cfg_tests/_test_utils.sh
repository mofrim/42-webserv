#!/usr/bin/env bash

# Utility funcs used in my shell-tests

CRLF="\r\n"

function sendHdrField() {
	if [ $# -ne 2 ]; then
		echo "need 2 args: msg fd"
		exit 1
	fi
	echo -en "$1$CRLF" >&$2
}

function finishReq() {
	if [ $# -ne 1 ]; then
		echo "need 1 arg: fd"
		exit 1
	fi

	echo -en "$CRLF" >&$1
}

# check if 2nd param, elem, is inside 1st param arr
function arrContains() {
	if [ $# -lt 2 ]; then
		echo "need 2 args: \"elem\" \"\${arr[@]}\""
		exit 1
	fi
	local cur match="$1"
	shift
	for cur; do
		[[ "$cur" == "$match" ]] && return 0
	done
	return 1
}

webserv="$(git rev-parse --show-toplevel)/webserv"
