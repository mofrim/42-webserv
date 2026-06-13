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

if [[ -n "$(grep 'macmac' <<< "$HOSTNAME")" ]]; then
	sed -i 's/frido/mofrim/g' ./cfgs/42tester.wsrv
	sed -i 's/frido/mofrim/g' ./www/42tester/wrapper.sh
elif [[ -n "$(grep 'jeanluc' <<< "$HOSTNAME")" ]]; then
	sed -i 's/mofrim/frido/g' ./cfgs/42tester.wsrv
	sed -i 's/mofrim/frido/g' ./www/42tester/wrapper.sh
fi

if pidof webserv; then
	echo ">> killing exsting webserv instance..."
	pkill webserv
fi

echo "now please run './webserv cfgs/42tester.sh' in another terminal!"
echo -n "press any key to continue..." && read -r


if ! pidof webserv; then
	echo "Error: Webserv is not running!"
	exit 1
fi

if [[ -z "$(grep -E 'jeanluc|macmac' <<< "$HOSTNAME")" ]]; then
	./tester http://localhost:1234
else
	nix-alien ./tester http://localhost:1234
fi
