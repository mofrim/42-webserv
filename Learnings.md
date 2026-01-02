# webserv Learnings

## Resources

- [Actually a multithreaded webserv but my entry-point to the epoll vs. poll
  story...](https://kaleid-liner.github.io/blog/2019/06/02/epoll-web-server.html)

- [A quite good Medium article about epoll and fd's and inodes](https://copyconstruct.medium.com/the-method-to-epolls-madness-d9d2d6378642)

- [Some epoll critique...](https://idea.popcount.org/2017-02-20-epoll-is-fundamentally-broken-12/)

- [The epoll man](https://man7.org/linux/man-pages/man7/epoll.7.html)

- [The epoll-ctl man](https://man7.org/linux/man-pages/man2/epoll_ctl.2.html)

- [Start of the blog-series about I/O-Multiplexing](https://idea.popcount.org/2016-11-01-a-brief-history-of-select2/) 

- [Killer Stack-Overflow thread on SO_REUSEADDR](https://stackoverflow.com/questions/14388706/how-do-so-reuseaddr-and-so-reuseport-differ)


## Networking, TCP/IP, socket-bind-listen-Business

### struct sockaddr_in

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

### network byte order

- for most protocols the standard byte order is **most significant first**, similar
  to **big-endian** in terms of **bit-ordering**.
- some protocols like *SMB* use little-endian
- attention: it is really the **byte** ordering! meaning the most significatn
  octet is transmitted first.

### The whole epoll-process

#### epoll vs. poll

major **TODO** !!!

#### epoll_wait is _always_ interrupted by signals

Whenever you hit `ctrl-c` even if you catch that signal using a signal-handler
`epoll_wait` will return -1 and `errno = EINTR` will be set!

### Notes about getaddrinfo

#### Where will we use this?

As explicitly noted in THE BOOK (*The Linux Programming Interface*) on p1206

    Converting host and service names to and from binary form (modern) The
    getaddrinfo() function is the modern successor to both gethostbyname() and
    getservbyname(). Given a hostname and a service name, getaddrinfo() returns
    a set of structures containing the corresponding binary IP address(es) and
    port number. Unlike gethostbyname(), getaddrinfo() transparently handles
    both IPv4 and IPv6 addresses. Thus, we can use it to write programs that
    don’t contain dependencies on the IP version being employed. All new code
    should use getaddrinfo() for con- verting hostnames and service names to
    binary representation. The getnameinfo() function performs the reverse
    translation, converting an IP address and port number into the corresponding
    hostname and service name. We can also use getaddrinfo() and getnameinfo()
    to convert binary IP addresses to and from presentation format.

So we really should use this function when creating and binding sockets! This
means it is probably a good idea to write a good wrapper function for the whole
traversing the linked list thing.

#### what is is in struct addrinfo?

```c
struct addrinfo {
      int              ai_flags;
      int              ai_family;
      int              ai_socktype;
      int              ai_protocol;
      socklen_t        ai_addrlen;
      struct sockaddr *ai_addr;
      char            *ai_canonname;
      struct addrinfo *ai_next;
  };
```
Most importantly: in order to retrieve / check the address we need to evaluate
the `struct sockaddr *ai_addr` part. But `struct sockaddr` is the generic struct
where port and address are completely mangled. In order to retrieve an address
using `inet_ntop` or something self-written we need to cast like in this
snippet:

```c
inet_ntop(rp->ai_family, &((struct sockaddr_in *)rp->ai_addr)->sin_addr,
          addrbuf, rp->ai_addrlen)
```

kinda ulgy?!


## Very general stuff

the `push_back` method of `std::vector` internally uses the copy constructor!!
so, if you haven't implemented this correctly for a class it won't work!


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

