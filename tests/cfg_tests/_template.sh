#!/usr/bin/env bash

# template

if [ $# -ne 2 ]; then
	echo "need 2 args: addr port"
	exit 1
fi

if [ ! -e ../_test_utils.sh ]; then
	echo "test-utils not found!"
	exit 1
else
	source ../_test_utils.sh
fi

set -u

OUTPUT="$(timeout -s INT 0.1s $webserv ./testcfgs/default_route0.wsrv)"

echo "Response:"
echo
echo "${OUTPUT[@]}"

has_default_route="$(echo "${OUTPUT[@]}" | grep -- "- \"/\"")"

if [[ -z "$has_default_route" || -z "$has_bla_route" ]]; then
	exit 1
fi
