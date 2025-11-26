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
