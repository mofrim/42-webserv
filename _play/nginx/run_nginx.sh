#!/usr/bin/env bash

if [ $# = 1 ]; then
	echo "reloading config"
	nginx -p ./ -e ./error.log -s reload
else
	nginx -p ./ -e ./error.log -g "daemon off;"
fi
