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
      listen 42.0.0.1:7777! # we will only allow stuff like this!
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


### Tokenization / Lexing

This is also called the *lexical analysis* of the config file.
