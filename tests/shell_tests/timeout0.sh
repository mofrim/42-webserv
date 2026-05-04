#!/usr/bin/env bash

# Test send a just one line too large header to the server.

set -u

CRLF="\r\n"

exec 3<>/dev/tcp/localhost/1111
echo -en "GET / HTTP/1.1$CRLF" >&3
echo -en "host: moep$CRLF" >&3
tmpfile=$(mktemp)
timeout 8s cat <&3 > $tmpfile &
sleep 6 && curl localhost:1111 &> /dev/null
exec 3<&-

cat $tmpfile

if [ -z "$(cat $tmpfile | grep 408)" ]; then
	rm $tmpfile
	exit 1;
fi


