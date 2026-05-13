# About mofrim's webserv

## General Ideas / Design Decisions

### use exceptions wherever possible... or not?!

maybe not... as clean-code in c++ prefers clear return value based error
handling over the possibly random occurence of exceptions

### do not implement file-based logging

this is not necessary and definitely something i can save time on

### we won't do IPv6!

as it is not demanded by the subject

### i will implement virtual servers!

this is some consequences... namely:

- during setup i need to check if some server already was initialized with the
  same addr:port combination _BUT_ as a different name
- what about the first server having bound the address `0.0.0.0`?
  Answer: any local address with the same port must have a different name.
  otherwise ignore the late-comer.


### we will have per vserver scoped error_pages. Not per route!

### route-paths won't be checked until requested! nginx is doing the same.

### we will demand one route per server! otherwise server cfg is invalid!

... or set default path to "/" and root = "html". yeah. i certainly will give
the possibility to have default values here. But certainly at least one route
has to be added to each VServer!

### i will only support `origin-form` as request target

i.e., targets have to start with a "/", otherwise `400`!

### the listen directive in cfgfile will have to be of format addr:port!

### if sth in the header is wrong -> 400 -> routes are not matched!

if errorpages in the server scope are set the corresponding page is tried to be
returned. if not found -> 404

### my current redirection logic is different from nginx!

in nginx, if `/redir` is a `return 301 /redir-target` a request like `GET
/redir/bla.html` will only bluntly redirect to `/redir-target` the way i handle
it now is the redirect will go to `/redir-target/bla.html`. so, the `targetPath`
is again appended to the redirected URL!
With the logic like this, you **need** to specify a new `root` for any
redirected path that corresponds to some real directory! F.ex. if req is for
`/redir/bla.html` but no root is given in `/redir-target` then default root is
used and so `./www/bla.html` is being tried.

---

## the initial webserv data-flow

- if we have a cmdline arg -> try to parse this as the configuration file. if
  not: start with default config

- parsing is happening in the config class but the lifetime of this class is
  restricted to `Webserv::getServersFromCfg()`. Why? Because we want to free the
  memory of the config after parse as we do not need it anymore.

- after parsing _all_ relevant information is stored in the `std::vector<Server>
  _servers` private member of Webserv-class.

## handling connections

this is the current main loop:

```cpp
	// the main loop
	while (_shutdown_server == false) {
		int nfds = _epoll.wait();
		if (nfds == -1 && errno != EINTR)
			throw(WebservRunException("epoll_wait failed"));
		for (int i = 0; i < nfds; ++i) {
			int currentFd = _epoll.getEventFd(i);
			if (_serverFds.find(currentFd) != _serverFds.end()) {
				int client_fd = _con.addNewClient(currentFd);
				_epoll.addClient(client_fd);
			}
			else {
				int requestStatus = _con.handleRequest(currentFd);
				if (requestStatus == -1)
					_epoll.removeClient(currentFd);
			}
		}
	}
	_epoll.closeEpollFd();
}
```

we personally find it more logically sound to add a client to a server. because
that is where they connect to!

but the 3 possible cases after `epoll_wait()` returns remain the same:

1) we got an I/O-event ready on a server-fd -> a new client wants to connect, so we
   assign the client to the correpsonding server _and_ start handling its
   requests (monitor keepalive, etc..)

2) we've got an event on a already existing client socket (so we also need a
   link between the client-socket the corresponding client _and_ its Server!). This is

  - if the event == `EPOLLIN` this is either a request (GET, POST, DELETE) coming in from the client

  - if the event == `EPOLLOUT` this is our response to a request

  - if the event == `EPOLLIN` and `read()` returns 0 this is a disconnect from
    the client -> remove the Client from the client list, the associated servers
    client-list, EPOLL_CTL_DEL from interest list the socket-fd and close
    socket-fd

  => as a conclusion: we should keep a client list around in Webserv-class. new
  clients are added to this list. the client stores a pointer to its server, and
  the server stores a pointer to the entry in the client list.

## how i want the connection handling to look like

  ```cpp
	while (_shutdown_server == false) {
		int nfds = _epoll.wait();
		if (nfds == -1 && errno != EINTR)
			throw(WebservRunException("epoll_wait failed"));

    // TODO: maybe... check if any connection should time out and close Remove
    // client if so.
    _checkKeepaliveConnectionTimeout();

    for (int eventIdx = 0; i < nfds; ++eventIdx) {
      int currentFd = _epoll.getEventFd(eventIdx);
      // 1. new connection
      if (_isServerFd(currentFd))
      {
        Server *srv = _getServerByFd(currentFd);

        // opens socket and so on. adds the Client to the servers Client list
        // returns a pointer to the new client from the servers client list
        Client *cli = srv->addClient(currentFd);

        _epoll.addClient(cli->getFd());
      }
      // 2. existing connection -> handle the 3 cases!
      else
      {
        // in order to do this efficiently store Fd's and pointers to servers in
        // a map!
        // is this save? is the mapping of fd -> server one-to-one? or can there
        // be a fd mapping to multiple servers or?
        Server *srv = _getServerByClientFd(currentFd);
        if (srv == NULL)
          throw (killer exception);

        // so in this case the server.handelEvent() routine will have to do much
        // work! also the remove-client thing as 

        if (srv->handleEvent(_epoll.getEvent(eventIdx)) == -1);
					_epoll.removeClient(currentFd);
      }
    }

  ```

## CGI Notes

From the [main resource](https://en.wikipedia.org/wiki/Common_Gateway_Interface):

- requests can look like: `webserv/cgi-bin/myscript.py?param1=moep&param2=miep`
- also they might look like:
  `webserv/cgi-bin/myscript.py/path/to/sth?and=some&params=yo`
- the additional path part is parsed into a env var `PATH_INFO` and the params
  are parsed to `QUERY_STRING` which are both passed to the scripts environment.
- in a POST request the message body is send to the `STDIN` of the script.

So, do i need to take anything here into account for my request handling
routine? It doesn't feel like.
