#!/usr/bin/env bash

# test if webserv is really adding default route "/"

if [ ! -e ../_test_utils.sh ]; then
	echo "test-utils not found!"
	exit 1
else
	source ../_test_utils.sh
fi

CFG="empty_server_block.wsrv"

set -u

OUTPUT="$(timeout -s INT 1.0 $valCmd $webserv ./testcfgs/$CFG; echo "ValExit=$?")"
echo "${OUTPUT[@]}"

valExit="$(echo "${OUTPUT[@]}" | grep ValExit)"
if [[ "$valExit" != "ValExit=0" ]]; then
	exit 1
fi

if [[ -z "$(echo $OUTPUT | grep 'Could not')" ]]; then
	exit 1
fi
