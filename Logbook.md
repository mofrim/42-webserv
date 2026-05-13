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

### 20:05

current possible TODOs:

- [ ] implement real virtual servers (serverName base routing)
- [ ] implement PUT req
- [ ] impl DEL req
- [ ] start with cfg parsing

## 2026-04-27

### 22:16

tomorrow:

- fix hanging cfg.
- fix `GET /../bla.md` problem test with nginx before
- make servernames and headers (except requline) case-insensitive!!! check that
  with RFC before again
- do further tests for real virtual server cfgs
- if v.s. feels lgtm bump version continue with other methods
  + `POST`
  + check what nginx does with GET-requests that have a body. or a
    Content-Length header and no body and so on.

- make up some kind of testing framework for requests and so on.
- is it possible to dump a `webser.pid` file to the working dir?

## 2026-04-28

### 08:48

Fixed a bug in `VServer::_findVirtualBuddy()`.Turns out i need another function
in `Socket` class to resolve server addrs alone. with that i can fill the
`_activeAddrPortPairs` map with the real IP. Actually... i only need this AND
the server name, as the `addr` specified in the Cfg in a `listen` directive is
only worth what it resolves to!

So... i need to refac VServer class again to only use `_activeAddrPortPairs`
after init.

## 2026-04-29

### 17:08

**Alrighty!** would say the above refac is done and virtual srvs is lgtm! but,
it definitely needs more testing! That is, try to construct weird server setups

What's next?

- [ ] fix the problem with URL / target path parsing -> should not be possible
  to access `../anything`!

- [ ] implement `POST`

Again some important citations from RFC:

      A client MUST send a Host header field (Section 7.2 of [HTTP]) in all
      HTTP/1.1 request messages.

=> so i have to check for `HTTP/1.1` and if Host is not send -> 400


- [ ] according to [this
  sectio](https://datatracker.ietf.org/doc/html/rfc9112#name-field-syntax) all
  field-names are case-insensitive!


## 2026-04-30

### 22:10

the main resource for everything about headers and methods is: [RFC
9110](https://datatracker.ietf.org/doc/html/rfc9110)

For example i just learned that my server won't support range-requests so we
should include `Accept-ranges:` none in our Responses to a GET

## 2026-05-07

### 15:22 - Finally starting with the parser!

- i will only listen directives like this `listen addr:port!`
- i will have comments!

## 2026-05-12

### 09:06 - Writing `Request::_matchRoute` ...

how to match a `/miep/moep/index.html` against `miep`?

### 10:36 - Major AHAAAAA!

    2026/05/12 10:35:37 [error] 60380#60380: *1 open() "././www/moep/moep"
    failed (2: No such file or directory), client: 127.0.0.1, server: localhost,
    request: "GET /moep HTTP/1.1", host: "localhost:1234"

with a nginx cfg like

    
    root ./www;
    location /moep {

      # Ahaa!!! nginx takes location-roots always relative to server root!
      root ./www/moep;
      index index-moep.html;
    }

and requested URL `localhost:1234/moep`. removing `root ./www/moep` leads to the
request being successful.

OH! it actually is more subtle:

    127.0.0.1 - - [12/May/2026:10:42:36 +0200] "GET /moep/ HTTP/1.1" 200 113 "-" "Mozilla/5.0 (X11; Linux x86_64; rv:149.0) Gecko/20100101 Firefox/149.0"
    2026/05/12 10:42:36 [error] 61688#61688: *1 open() "././www/favicon.ico" failed (2: No such file or directory), client: 127.0.0.1, server: localhost, request: "GET /favicon.ico HTTP/1.1", host: "localhost:1234", referrer: "http://localhost:1234/moep/"
    127.0.0.1 - - [12/May/2026:10:42:36 +0200] "GET /favicon.ico HTTP/1.1" 404 153 "http://localhost:1234/moep/" "Mozilla/5.0 (X11; Linux x86_64; rv:149.0) Gecko/20100101 Firefox/149.0"
    127.0.0.1 - - [12/May/2026:10:43:02 +0200] "GET /moep HTTP/1.1" 301 169 "-" "Mozilla/5.0 (X11; Linux x86_64; rv:149.0) Gecko/20100101 Firefox/149.0"
    127.0.0.1 - - [12/May/2026:10:43:02 +0200] "GET /moep/ HTTP/1.1" 200 113 "-" "Mozilla/5.0 (X11; Linux x86_64; rv:149.0) Gecko/20100101 Firefox/149.0"

so if the request is only for `/moep` a redirect `301` to `/moep/` is send.

with curl this is directly visible:

```console
$ curl -v localhost:1234/moep

* Host localhost:1234 was resolved.
* IPv6: ::1
* IPv4: 127.0.0.1
*   Trying [::1]:1234...
* connect to ::1 port 1234 from ::1 port 50648 failed: Connection refused
*   Trying 127.0.0.1:1234...
* Established connection to localhost (127.0.0.1 port 1234) from 127.0.0.1 port 46066 
* using HTTP/1.x
> GET /moep HTTP/1.1
> Host: localhost:1234
> User-Agent: curl/8.19.0
> Accept: */*
> 
* Request completely sent off
< HTTP/1.1 301 Moved Permanently
< Server: nginx/1.30.0
< Date: Tue, 12 May 2026 08:45:39 GMT
< Content-Type: text/html
< Content-Length: 169
< Location: http://localhost:1234/moep/
< Connection: keep-alive
< 
<html>
<head><title>301 Moved Permanently</title></head>
<body>
<center><h1>301 Moved Permanently</h1></center>
<hr><center>nginx/1.30.0</center>
</body>
</html>
* Connection #0 to host localhost:1234 left intact
```

### 13:50 - still processing request processing...

<https://nginx.org/en/docs/http/request_processing.html>

### 19:03 - the plan for route-matching

given a route `route /moep {}`

given a route `route /moep/index.html {}`

**i somewhat tend towards only doing longest match routing. no fancy trailing
slash stuff. or, only redirection if dir route exists. meaning, only redirect
/moep to /moep/ if /moep/ exists.**

**no. if `/moep` is requested and `/moep` as a route exists, there will be no
redirect but simply the route `/moep` will be selected and the index be served.

if `/moep/bla.html` is requested. the `/moep` route will be selected and
`bla.html` will be searched for in whatever root the route has. this implies i
will have to seperate the requested path from the detected route so that i can
pass on the file which has to be opened to `_getBody` (so far...).

## 2026-05-13

1) If a Header is malformed, sth like `asdlkfjalsdhjflksadj` instead of `Host:
   asdasdas` 400 is returned and the possible route is NOT matched. However IFF
   a corresponding errorPage exists on server level, this is tried to be served.
   If that errorpage's file does not exist a 404 with the corresponding
   errorPage's path is returned.

So, how and where do i inject the errorPage into the Response?
