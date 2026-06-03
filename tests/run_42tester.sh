#!/usr/bin/env bash
set -e

repoRoot="$(git rev-parse --show-toplevel)"
cd $repoRoot

if [ ! -e ./tester ]; then
	curl -o ./tester https://cdn.intra.42.fr/document/document/47655/tester
	chmod +x ./tester
fi

if [ ! -e ./www/42tester/cgi_tester ]; then
	curl -o ./www/42tester/cgi_tester https://cdn.intra.42.fr/document/document/47656/cgi_tester
	chmod +x ./www/42tester/cgi_tester
fi

if ! pidof webserv; then
	echo "Plz, start webserv with 42tester cfg before this!"
	exit 1
fi

if [[ -z "$(grep -E 'jeanluc|macmac' <<< "$HOSTNAME")" ]]; then
	./tester http://localhost:1111
else
	nix-alien ./tester http://localhost:1111
fi


