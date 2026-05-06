# A Collection of Nginx default responses to certain requests

Commands used:

- `exec 3<>/dev/tcp/localhost/2222`
- `echo -en "..." 1>&3`
- `cat <&3` to read responses


## `echo -en "bla\r\n\r\n" 1>&3`

```
HTTP/1.1 400 Bad Request
Server: nginx/1.28.2
Date: Thu, 23 Apr 2026 15:43:37 GMT
Content-Type: text/html
Content-Length: 157
Connection: close

<html>
<head><title>400 Bad Request</title></head>
<body>
<center><h1>400 Bad Request</h1></center>
<hr><center>nginx/1.28.2</center>
</body>
</html>
```

## `echo -en "GET / HTTP/1.1\r\n\r\n" 1>&3`

```
HTTP/1.1 400 Bad Request
Server: nginx/1.28.2
Date: Thu, 23 Apr 2026 15:47:39 GMT
Content-Type: text/html
Content-Length: 157
Connection: close

<html>
<head><title>400 Bad Request</title></head>
<body>
<center><h1>400 Bad Request</h1></center>
<hr><center>nginx/1.28.2</center>
</body>
</html>
```

## `echo -en "GET / HTTP/1.1\r\nHost: localhost\r\n" 1>&3` && `echo -en "\r\n" 1>&3`

```
HTTP/1.1 200 OK
Server: nginx/1.28.2
Date: Thu, 23 Apr 2026 15:50:31 GMT
Content-Type: text/html
Content-Length: 101
Last-Modified: Wed, 24 Dec 2025 12:04:29 GMT
Connection: keep-alive
ETag: "694bd6cd-65"
Accept-Ranges: bytes

<html>
  <head>
    <title>Hello!</title>
  </head>
  <body>
    This is the body!
  </body>
</html>
```

## AHA! A very interesting timeout case!

In bash-session A run `echo -en "GET / HTTP/1.1\r\nHost: localhost\r\nBlarg:
bloep\r\n" 1>&3` which is an incomplete req due to the final `\r\n` missing.

Nothing happens when waiting with `cat <&3` in A.

In bash-session B run `echo -en "GET /moep HTTP/1.1\r\nHost: localhost\r\nBlarg:
bloep\r\n\r\n" 1>&3` or any other valid or invalid complete req. Only then the
incomplete req in session A is terminated with 408
