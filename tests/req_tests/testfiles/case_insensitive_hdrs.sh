#!/usr/bin/env bash

# Test send a just one line too large header to the server.

set -u

CRLF="\r\n"

exec 3<>/dev/tcp/localhost/1111
echo -en "GET / HTTP/1.1$CRLF" >&3
echo -en "host: moep$CRLF" >&3
echo -en "$CRLF" >&3
RESPONSE="$(timeout 0.1s cat <&3)"
exec 3<&-

if [ -z "$RESPONSE" ]; then
	exit 1;
fi

