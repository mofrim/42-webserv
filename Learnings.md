# webserv Learnings

## struct sockaddr_in

`struct sockaddr_in` is short for *Socket Address Internet*. There is also
`struct sockaddr_in6` for IPv6 Addresses. In a call to bind like this:

```cpp
bind(bind(sockfd, (struct sockaddr*)&addr_in, sizeof(addr_in));)
```

the struct gets casted to the generic `sockaddr` struct. This is for portability
and extensibility reason as there there is also `unix` socket and `packet`
sockets but they can all use the same `bind()` function.

Here the relevant quote from `sys/socket.h`:

```c
struct sockaddr {
    sa_family_t     sa_family;      /* Address family */
    char            sa_data[];      /* Socket address */
};

struct sockaddr_storage {
    sa_family_t     ss_family;      /* Address family */
};

typedef /* ... */ socklen_t;
typedef /* ... */ sa_family_t;

```

and from `netinet/in.h`

```c
struct sockaddr_in {
    sa_family_t     sin_family;     /* AF_INET */
    in_port_t       sin_port;       /* Port number */
    struct in_addr  sin_addr;       /* IPv4 address */
};

struct sockaddr_in6 {
    sa_family_t     sin6_family;    /* AF_INET6 */
    in_port_t       sin6_port;      /* Port number */
    uint32_t        sin6_flowinfo;  /* IPv6 flow info */
    struct in6_addr sin6_addr;      /* IPv6 address */
    uint32_t        sin6_scope_id;  /* Set of interfaces for a scope */
};

struct in_addr {
    in_addr_t s_addr;
};

struct in6_addr {
    uint8_t   s6_addr[16];
};

typedef uint32_t in_addr_t;
typedef uint16_t in_port_t;

...
```

## network byte order

- for most protocols the standard byte order is **most significant first**, similar
  to **big-endian** in terms of **bit-ordering**.
- some protocols like *SMB* use little-endian
- attention: it is really the **byte** ordering! meaning the most significatn
  octet is transmitted first.

## The whole bind-epoll-process

major **TODO** !!!

### epoll_wait is _always_ interrupted by signals

Whenever you hit `ctrl-c` even if you catch that signal using a signal-handler
`epoll_wait` will return -1 and `errno = EINTR` will be set!

## Very general stuff

the `push_back` method of `std::vector` internally uses the copy constructor!!
so, if you haven't implemented this correctly for a class it won't work!


## epoll vs. poll

## a learning about iterating over a list and removing things

this code, is obviously _very_ problematic: 

  ```cpp
	for (std::list<Client>::iterator it = _clients.begin(); it != _clients.end();
			it++)
	{
		if (it->getFd() == fd)
			_clients.erase(it);
	}
  ```
  if the only list-member is erased the iterator run into memory locations where
  it shouldn't be -> **segfault**.

  better do this:
  ```cpp
  std::list<Client>::iterator it = _clients.begin();
	while(it != _clients.end())
	{
		if (it->getFd() == fd)
			it = _clients.erase(it);
    else
      ++it;
	}
  ```
  why the pre-increment operator `++it`? the pre-increment operator does not
  create a copy of the iterator and returns this not-yet-incremented copy. so
  using `++it` is just about memory-efficiency.


### Resources

- [Actually a multithreaded webserv but my entry-point to the epoll vs. poll
  story...](https://kaleid-liner.github.io/blog/2019/06/02/epoll-web-server.html)

- [A quite good Medium article about epoll and fd's and inodes](https://copyconstruct.medium.com/the-method-to-epolls-madness-d9d2d6378642)

- [Some epoll critique...](https://idea.popcount.org/2017-02-20-epoll-is-fundamentally-broken-12/)

- [The epoll man](https://man7.org/linux/man-pages/man7/epoll.7.html)

- [The epoll-ctl man](https://man7.org/linux/man-pages/man2/epoll_ctl.2.html)

- [Start of the blog-series about I/O-Multiplexing](https://idea.popcount.org/2016-11-01-a-brief-history-of-select2/) 
