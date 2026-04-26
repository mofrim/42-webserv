# mofrim's webserv logbook

## 2026-04-17

Clarified some things around `bind` and how to implement virtual servers and the
whole server cfg parsing process in general.

Oh, dang! What was my Idea with the `Socket` class again?! What i know for sure:
i do not want exception throwing ctors!!!

OK, i did what i had to do and made Socket a utility class. We'll see, if i
regret the work i spend on this. gn8

## 2026-04-18

I wanted to use `Socket` class in `Webserv::initDefaultCfg2` /  `Server::init`.
However i must admit again that i first need to clarify the case with multiple
interface:port pairs per server... And where i hook in the virtual-server
feature (i.e. if two servers share the same interface:port they can still be
distinguished by `server_name`). So, here is the plan:

- What will be the data-structure for interface:port combinations in
  `ServerCfg`?
  + `std::vector<std::pair<str, u16> >`?
  + `std::map<str, u16>` has the (dis-?!?!)advantage that there can only be one
    `10.0.0.1:1234` and not at the same time  `10.0.0.1:4321`
  + `std::multimap<str, u16>`
    * advantage: everything is possible
    * dis-advantage: iteration is difficult... but feasible
  + `std::map<str, std::vector<u16> >`
  + `std::map<str, std::list<u16> >`
  + `std::map<str, std::set<u16> >`
    => **that's it!!!!!**
    because we do not need any duplicate ports per addr!

Now i will start and add this `std::map<str, std::list<u16> >` as
`ServerCfg::_interfaces` along-side with the existing 1-dim `_port` and `_host`

### 14:00

So far so good. Settled with `std::map<str, std::set<u16> >`  for
`ServerCfg::_interfaces`. Implemented basic getter  setters. Next things:

- write a couple more tests for `ServerCfg::_interfaces`. but do not spend too
  much time on that!
- proceed with integration into all other classes mainly: `Server`

## 2026-04-20

### 11:35

Alright what is to be done today?

- [x] the multiple interfaces per VServer feat is implemented. now remove the
  host port variables from class

- [x] keep a list with all the resolved ip addr - port pairs in the VServer
  class therefore make `Socket::bindSocket` return a pair of this

- [ ] the config file which is handed to the VServer object in ctor does not
  have to be imported into the class. Therefore import all the settings from the
  `VServerCfg` into the `VServer` instance.

- [ ] the `Webserv::_getServerByFd` has to be modified to take real vservers
  which only differ by `server_name` into account!

- [ ] clarify at which steps, which syntax or error checking will be done?
  Espescially for the `VServerCfg`!!!

  + F.ex., certainly for the socket things checks can only be done in during
    `VServer::init` but for error_pages we can delay this until a error_page is
    really requested?! And then fallback to some kind of default error_page?

  + certainly the validity of addresses and ports in the `_interfaces` map
    should already be checked! not for existence or availability but for
    syntactical correctness!

### 16:21

About error_pages

- i will need a `ErrorPage` class which has these private variables:
  + the code itself
  + a path
  + and a fallback string if path is not readable or empty, BUT: this string
    should be stored somewhere globally bc it would be insane to make this a
    complete distinct obj in every vserver!
- then i will have a `std::map<u16, ErrorPage>` which will be used to select the
  corresponding error page and via a public method
  `ErrorPage::getErrorResponseStr` or sth like that,
- somewhere before that i will need a function that specifies which Status will
  be emitted. Therefore i will need to decide which error codes i will need to
  support in the end.
  

**No!** i will construct the error-pages on the fly using a template and the
corresponding string to insert! This is definitely the most (mem-)efficient
way! as there does not have to be any global table.

**Yes!**  i did it! `ErrPages` Utility class is now realized

## 2026-04-21

Oh dang! The virtual servers. What will i need to do in order to check if one
server's interface is already initialized by another server? For ipaddrs other
than 0.0.0.0 this would be simple. but how to check for 0.0.0.0, 127.0.0.1 and
stuff like that? i would need some sort of procedure to check if an interface is
a local interface. 

OK! Now i have got a plan:

- `EADDRINUSE` in `VServer::init` resp. `Socket::bindSocket` will lead to the
  addr:port pair being added to a `_addrInUseInterfaces` `std::map` in `VServer`
- Then in `Webserv::_setupSingleServer` i check if `_addrInUseInterfaces.size()
  != 0`, if so, search all so far setup vservers for the addr:port
- compare server names. if they are the same: drop the server that was not the
  first to bind to this interface. check if the dropped server has any
  successfully setup ifaces left. if not: emit warning / error set `setupFailed`
- if the server_names differ get the FD of the corresponding iface and add the
  second server to it.

## 2026-04-22

What i just learned again today: a valid Request consists of a Header which is
multiple lines ended by `\r\n` and the whole header terminated by `\r\n\r\n`
which is the `\r\n` for the last line and an empty line, i.e. just `\r\n`

**THE MAIN RESOURCE FOR ALL HTTP RELATED STUFF::::....**

[DA ONE AND ONLY RFC](https://datatracker.ietf.org/doc/html/rfc9112)

- [Request Line Format](https://datatracker.ietf.org/doc/html/rfc9112#name-request-line)

- [A whole section on msg parsing](https://datatracker.ietf.org/doc/html/rfc9112#section-2.2)

## 2026-04-23

from the RFC:

    The normal procedure for parsing an HTTP message is to read the start-line
    into a structure, read each header field line into a hash table by field
    name until the empty line, and then use the parsed data to determine if a
    message body is expected. If a message body has been indicated, then it is
    read as a stream until an amount of octets equal to the message body length
    is read or the connection is closed.

### 10:50 - so amazed by bash once again!

YESSSSS! F****ing YESSS!!! I already knew about `bash` network programming caps
but this:

- open a tcp connection to addr:port
  
  ```bash
  exec 3<> /dev/tcp/addr/port
  ```

- write sth to that conn:

  ```bash
  echo "hi!" 1>&3
  ```

- close the conn:

  ```bash
  exec 3<&-
  ```

so now i can do very fine-grained testing of my connection and request
handling - c'est fantastique!

### 12:39

So, it is time to stop think about the whole connection / request handling data
flow.

What is happening if a client sends a request to one vsrv (the easy case where
is only one possible server responsible)

1. Client connects in `Webserv::run` and via `cli = vsrvs[0]->addClient(currentFd);`
  Note: this call adds the client to the vsrv's `std::map<int, Client *>
  _clients` map AND also adds the corresponding vsrv to the Client as the one
  and only `_vsrv`.

2. the client's fd is added to epolls interest list and to
   `Webserv::_fdClientMap`

3. When the clients sends a req the `else`-branch is being run

4. in the `else`-branch we determine the `Client` and `VServer` resonsible and
   the vsrv starts the request handling.

   **IMPORTANT NOTE**: the vsrv is determined via `cli->getVsrv()`! This might
   already be a hint that we do not need to keep the aggregated clients info in
   the `VServer` class.

5. the vsrv launches `vsrv->handleEvent(_epoll.getEvent(eventIdx), cli)` from
   then on the servers `RequestHandler` takes control

What question that remains: Is it possible that a client has 2 independent
pending requests? Meaning, 2 requests which have not yet been entirely received.

### 13:20 - implementing some proper request parsing

... kind of done!

## 2026-04-24

a possible sensible TODO for today would be to properly implement the connection
closing and handling in case of certain statusCodes and regarding timeout.

- maybe i should also have [send timeout](https://nginx.org/en/docs/http/ngx_http_core_module.html#send_timeout)

- likewise important: [client body and header timeout](https://nginx.org/en/docs/http/ngx_http_core_module.html#client_body_timeout)
  interesting about this: the timeout is only measured between 2 consecutive
  reads from the client.

## 2026-04-25

- implement timeouts

## 2026-04-26

maybe even implement a simple form of rate-limiting
