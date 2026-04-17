#!/usr/bin/env bash

if [ $# = 1 ] && [ -e $1 ]; then
	nginx -p ./ -c $1
else
	nginx -p ./ -e ./error.log
fi
