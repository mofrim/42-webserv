# Webserv Programming Reference

Her i want to collect quick-access overview info for all the many functions and
options related to network programming on Linux.

## `int socket(int domain, int type, int protocol)`

`#include <sys/socket.h>`

- `int domain`: the adress family used in communications domain
  + `AF_INET` for IPV4
  + `AF_INET6` for IPV6
  + `AF_UNIX` for a unix socket

- `int type`: `SOCK_STREAM`, `SOCK_DGRAM`
  + can also be OR'd together with f.ex. `SOCK_NONBLOCK` which saves acall to
    `fcntl`
  + `SOCK_CLOEXEC` is another interesting one which makes the socket close

- `int protocol`: TODO


## `int setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len)`

`#include <sys/socket.h>`

- `int socket`: the sockets fd
- `int level`:
  + `SOL_SOCK`: 
- `int option_name`:
  + `SOCK_NONBLOCK`

## `struct in_addr`

`#include <netinet/in.h>`

```c
struct in_addr {
           in_addr_t s_addr;
       };
```
