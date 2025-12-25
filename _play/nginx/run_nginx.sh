#!/usr/bin/env bash

if [ $# = 1 ]; then
	echo "reloading config"
	nginx -p /home/mofrim/tmp/nginx -e ./error.log -s reload
else
	nginx -p /home/mofrim/tmp/nginx -e ./error.log
fi
