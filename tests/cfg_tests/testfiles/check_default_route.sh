#!/usr/bin/env bash

# test if webserv is really adding default route "/"

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
# echo "${OUTPUT[@]}"
has_default_route="$(echo "${OUTPUT[@]}" | grep -- "- \"/\"")"
has_bla_route="$(echo "${OUTPUT[@]}" | grep -- "- \"/bla\"")"

if [[ -z "$has_default_route" || -z "$has_bla_route" ]]; then
	exit 1
fi

OUTPUT="$(timeout -s INT 0.1s $webserv ./testcfgs/default_route1.wsrv)"
has_default_route="$(echo "${OUTPUT[@]}" | grep -- "- \"/\"")"
has_bla_route="$(echo "${OUTPUT[@]}" | grep -- "- \"/bla\"")"

if [[ -z "$has_default_route" || -n "$has_bla_route" ]]; then
	exit 1
fi
