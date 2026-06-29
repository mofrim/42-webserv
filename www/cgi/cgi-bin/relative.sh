CRLF=$'\r\n'

function sendHdrField() {
	echo -en "$1$CRLF"
}

function finishReq() {
	echo -en "$CRLF"
}

# msg="query: $QUERY_STRING"
msg="pwd: $PWD"
len="${#msg}"

sendHdrField "Content-Type: text/plain"
sendHdrField "Content-Length: $len"
finishReq
echo -en "$msg"
