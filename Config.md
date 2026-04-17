# Notes And Clarification Regarding Config File

## From Subject.pdf

- [ ] Define all the interface:port pairs on which your server will listen to
  (defining mul- tiple websites served by your program).

- [ ] Set up default error pages.

- [ ] Set the maximum allowed size for client request bodies.

- [ ] Specify rules or configurations on a URL/route (no regex required here),
  for a website, among the following:

- [ ] List of accepted HTTP methods for the route.

- [ ] HTTP redirection.

- [ ] Directory where the requested file should be located (e.g., if URL
  /kapouet is rooted to /tmp/www, URL /kapouet/pouic/toto/pouet will search for
  /tmp/www/pouic/toto/pouet).

- [ ] Enabling or disabling directory listing.

- [ ] Default file to serve when the requested resource is a directory.

- [ ] Uploading files from the clients to the server is authorized, and storage
  location is provided.

## Structured Overview Of Directives

### `vserver`

type: `BLOCK`

scope: `GLOBAL`

example:

```
vserver {
  server_name moep!
  listen 1.1.1.1:1111
  ...
}
```

rules:
  - must be followed by `START_BLOCK` and end with `END_BLOCK`

todos:
  - what to do with an empty server-block? fill in default values? error?

### `server_name`

type: `KEYWORD`

scope: `VSERVER`

```
server_name: moep.de
# or:
server_name: moep
# or:
server_name: miep.moep.de


# but never
# server_name: moep.de/index.html
# or
# server_name: moep.miep.marp.de
# => only _ONE_ subdomain allowed!
```

rules:
  - 




## Clarification along with subject.pdf

In the configuration file, you should be able to:


=> we need multiple server blocks
=> per server we can have multiple interface:port

**Question**:
will i require at least one `listen`-directive per server block?
Yes!

So the structure i derive from this is:

```
server {
  listen localhost:80!
  listen 10.0.0.1:999!
}
```

- There will be only IPv4/domain.name:Port pairs allowed !!!
- So, sth like `listen localhost` or `listen :80` like in nginx is invalid
  syntax!

I also want a `server_name` here! so:

```
server {
  server_name "kallehoinz"!
  listen localhost:80!
  listen 10.0.0.1:999!
}
```

- `server_name` restrictions: string **without whitespaces** containing only
  alphanumerics and dots.

---



## A Full-Featured Config-File

```nginx

# multiple servers!
server {
  server_name webserv;

  # we want error pages only in server-context, not in `route`-context
  error_page 404 /404.html;
  error_page 501 502 503 /50x.html;

  # setting max_body_size like 1kb, 10000 (bytes).
  # 1mb is default.
  # too large -> error 413
  client_max_body_size 1m;

  # route-based rules
  route /bla {

    # accepted methods for this location.
    methods GET, POST, DELETE;

    # http redirection
    # i personally would call this `redirect` in our case
    # TODO: find out about 30x status codes!
    return 301 https://bla.de$request_uri;

    # the root of all
    # ... by absolute path
    root /var/www/html;
    # relative path
    root ./moep;
    # or
    root moep/miep;

    # dir listings
    autoindex on;

    # default file to serve if requested file is a dir
    # also in global context
    index index.html;

    # uploading files?! in this case goes to location...
    sendfile on;
    # in server-context we might even support
    # sendfile /path/to/upload/dir;
  }

  # TODO: CGI ...

}
```
