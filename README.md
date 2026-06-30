*This project has been created as part of the 42 curriculum by fmaurer*

# Webserv

## Description

This project's goal is to create a single-threaded minimal web-server which uses
non-blocking I/O for all socket and pipe related operations. The whole program
should be written in C++98 standard running on Linux. To limit the scope and
time requirements for this project only a subset of the full-feature list of a
modern web-server, like e.g. the well-known *Nginx*, was to be implemented. That
is:

- Supported methods: GET, POST, DELETE
- Extra feature: a simple CGI support
- Mostly configurable via a external config file
- Default status pages and the ability to overwrite them in configuration
- Path-based routing, with the possibility to specify route-scoped rules
- File-Upload
- Ability to listen on multiple interfaces at the same time + basic Host-header
  based virtual-host request-routing
- Mostly RFC 9112 - HTTP/1.1 conform
- IPv4 adress handling

Besides the technical and Linux/Unix system-programming-knowledge to implement basic
TCP/IP networking via sockets, the features provided by the Linux-Kernel for
I/O-Multiplexing, namely the *Epoll* system-call set were of key interest for this
project. My reference web-server, Nginx, also uses Epoll in order to realize
non-blocking I/O. Also this method is the currently recommended one, as older
system like `select(2)` and `poll(2)` are being out-performed by Epoll, at least
for large numbers of monitored file descriptors.

Handling the read/write-operations in the Epoll-System was from a programming
perspective the most challenging part of the whole project.


## Instructions

The program was tested with the compiler `clang v12.6`. This and *GNU make* are
the only prerequisites for building the project from source. To start the build
simply run `make` in the repo-root. This will generate an executable called
`webserv`. When launched without any command-line-arguments a default server
configuration is loaded which would serve any GET-requested files under `./www`.

To load a custom configuration file pass it as the single argument to `webserv`.
Here is a configuration file with all supported directives:

```
timeoutCgi 60!
timeoutReq 5!
server {
  serverName nginWebserv!
  listen localhost:1234!
  root ./some/dir!
  errorPage 404:/path/to/404.html!
  maxBodySize 12345!

  route /upload {
    autoindex on!
    upload /path/from/root/to/upload-dir!
    methods GET!
  }

  route /cgi {
    index index.sh!
    cgi sh:/bin/bash!
    methods POST GET!
  }

  route /42 {
    redirect 301:https://42.fr!
  }
}
```

The parsing is implemented in a very strict fashion. I.e., if there is something
wrong in a server-block, the whole server-block the whole server-block will be
rejected. On certain errors the whole config-file will be rejected. It was tried
to make the error messages as descriptive as possible in order to assist the
user with fixing the config.

### Other Relevant Instructions

- For launching the **42-Tester** run `make 42tester` ;)
- For running my own tests i used during development enter `make allMyTests`
- with `make dbg1` / `make dbg2` it is possible to increase the verbosity of
  debugging output. This can also be interesting to understand the whole
  processing.

## Resources

- `man 2 waitpid`
- `man 2 socket`
- `man 3 sockaddr`
- `man 3 sockaddr`
- `man 2 epoll_ctl`
- `man 2 epoll_create`
- `man 2 bind`
- `man curl`
- [All the HTTP StatusCodes](https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Status)
- [The one and only CPP Reference - Legacy Edition, for true C++98 geeks](https://legacy.cplusplus.com/reference/)
- [RFC 9112 - HTTP/1.1](https://datatracker.ietf.org/doc/html/rfc9112)
- [RFC 9110 - HTTP Semantics](https://datatracker.ietf.org/doc/html/rfc9110)
- [RFC 3986 - URI](https://datatracker.ietf.org/doc/html/rfc3986)
- [RFC 3875 - CGI](https://www.rfc-editor.org/rfc/rfc3875)
- [Actually a multithreaded webserv but my entry-point to the epoll vs. poll
  story...](https://kaleid-liner.github.io/blog/2019/06/02/epoll-web-server.html)
- [A quite good Medium article about epoll and fd's and inodes](https://copyconstruct.medium.com/the-method-to-epolls-madness-d9d2d6378642)
- [Some epoll critique...](https://idea.popcount.org/2017-02-20-epoll-is-fundamentally-broken-12/)
- [The epoll man](https://man7.org/linux/man-pages/man7/epoll.7.html)
- [The epoll-ctl man](https://man7.org/linux/man-pages/man2/epoll_ctl.2.html)
- [Start of the blog-series about I/O-Multiplexing](https://idea.popcount.org/2016-11-01-a-brief-history-of-select2/) 
- [Killer Stack-Overflow thread on SO_REUSEADDR](https://stackoverflow.com/questions/14388706/how-do-so-reuseaddr-and-so-reuseport-differ)
- [Nginx config about error pages](https://nginx.org/en/docs/http/ngx_http_core_module.html#error_page)
- [Nginx: alpabetical index of all directives](https://nginx.org/en/docs/dirindex.html)

### AI Usage

In this project AI was used for the following purposes:

- Asking questions about protocol special cases. For example question about when
  which status code should be returned.
- Getting an overview about certain topics. For example i asked a chatbot about
  the pros and cons of the different I/O-multiplexing methods in the very
  beginning but then read more about on primary resources throughout the web.
- Sometimes bug-hunting.
