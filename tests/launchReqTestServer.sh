#!/usr/bin/env bash

# helper script for launching webserv in the same env like in testfiles

repoRoot="$(git rev-parse --show-toplevel)"
cfgDir="$repoRoot/tests/_cfgs"
wwwDir="$repoRoot/tests/_www"
webserv="$repoRoot/webserv"

if [ $# -ne 1 ]; then
	echo "need 1 arg: the filename of the test-cfg to run!"
	exit 1
fi

cfgname="$1"

cd $repoRoot/tests/req_tests/testfiles

$webserv $cfgDir/$cfgname
