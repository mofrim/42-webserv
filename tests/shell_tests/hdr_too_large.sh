#!/usr/bin/env bash

# Test send a just one line too large header to the server.

set -euo pipefail

CRLF="\r\n"

exec 3<>/dev/tcp/localhost/1111
echo -en "GET / HTTP/1.1$CRLF" >&3
for ((i=0;i<499;i++)); do
	echo -en "Dummy: header$CRLF" >&3
done
echo -en "$CRLF" >&3
RESPONSE="$(head -n 20 <&3 |grep 200)"
echo "Response: $RESPONSE"
exec 3<&-

if [ -z "$RESPONSE" ]; then
	exit 1;
fi

