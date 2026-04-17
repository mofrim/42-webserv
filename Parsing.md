# Notes about implentation of cfg file parsing

## Basic stuff

1) Our configuration file format will follow nginx:

  ```nginx
  # in global context maybe configure logging file path only like
  logfile /path/to/logfile!
  server {
      listen 8080;
      server_name serverA!
      root /var/www/htmlA!
      location / {
          index index.html!
      }
  }

  server {
      listen 1234!
      listen 9999! # yes! we will have multiple ports
      listen 42.0.0.1:7777! # ... and maybe even this.
      server_name serverB!
      root /var/www/htmlB!
      location / {
          index index.html!
      }
  }

  ```
  yes! _we will use the ! as directive delimiter_!!!

  2) We will use a **state machines** for both tokenization _and_ parsing!

  3) we will build an AST from the tonkenized cfg file which we finally write to
     the `ServerCfg`s in the `Config` class.

## Structure / Sequence / Workflow

1) **Tokenization / Lexing**: 
  + **Tokens**:
    - `TOKEN_BLOCK_START` (`{`)
    - `TOKEN_BLOCK_END` (`}`)
    - `TOKEN_DIRECTIVE` (`listen`, `server_name`, etc.)
    - `TOKEN_STRING` (`webserv.42.fr`)
    - `TOKEN_NUMBER` (`8080`)
    - `TOKEN_SEMICOLON` (`;`)
  + **Errors**: 
    - unexpected chars
    - eof while parsing a token
    - ???
2) **Parsing**
  + **Errors**:
    - unexpected token: grammar / context check (f.ex. no `server`-context token
      in `location`-context)
    - missing semicolon
    - unmatched brackets
3) **Semantic Check**
  + **missing required directives**: `server` block must have a `listen`
    directive.
  + **invalid values**: port numbers must be within the valid range (1-65535).
  + **duplicate directives**: multiple `listen` directives in the same block.

## Clarification of technical terms and demands

### Clarification of demanded specs in config file

```nginx

# multiple servers!
server {

}

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

### Clarification along with subject.pdf

In the configuration file, you should be able to:

    Define all the interface:port pairs on which your server will listen to (defining mul-
    tiple websites served by your program).

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


• Set up default error pages.
• Set the maximum allowed size for client request bodies.
• Specify rules or configurations on a URL/route (no regex required here), for a
website, among the following:
◦List of accepted HTTP methods for the route.
◦HTTP redirection.
◦Directory where the requested file should be located (e.g., if URL /kapouet
is rooted to /tmp/www, URL /kapouet/pouic/toto/pouet will search for
/tmp/www/pouic/toto/pouet).
◦Enabling or disabling directory listing.
◦Default file to serve when the requested resource is a directory.
◦Uploading files from the clients to the server is authorized, and storage location
is provided.

### Tokenization / Lexing

This is also called the *lexical analysis* of the config file.
