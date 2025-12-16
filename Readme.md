# project ideas

## general ideas

- **use exceptions wherever possible!!!**

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









