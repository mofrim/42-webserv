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

### Tokenization / Lexing

This is also called the *lexical analysis* of the config file.
