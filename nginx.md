# Learning about nginx

## quicknotes

- to start nginx in foreground when configured as daemin use: `-g "daemon
  off;"`. this has the same effect as adding

  ```nginx
  daemon off;
  ```
  to the configuration file.

## how nginx handles things

### how nginx initializes with minimal config

### how nginx routes a request to the correct server

**This is rather important bit of information as we are currently not getting
this right!!!**

So we got a major TODO here! In principle a nginx config like this is possible
and even common practice:

```nginx
server {
    listen 80;
    server_name example.com;
    # Configuration for example.com
}

server {
    listen 80;
    server_name another.com;
    # Configuration for another.com
}
```

When a request arrives, nginx uses the following logic to select the server
block:

IP and Port Matching:

  nginx first selects all server blocks that match the IP address and port the
  request was received on (as defined by the listen directive).

Host Header Matching:

  From the matching server blocks, nginx then selects the one whose server_name
  best matches the Host header in the request. The matching is done as follows:

Exact name match (e.g., server_name example.com; matches Host: example.com).

Longest wildcard match at the beginning (e.g., server_name *.example.com;
matches Host: foo.example.com).

Longest wildcard match at the end (e.g., server_name example.*; matches Host: example.org).

First matching regular expression (if any).

**If no server_name matches, the default server for the IP/port is used (the first
server block in the configuration file for that IP/port, unless explicitly
marked as default_server).**




