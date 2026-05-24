# webserv TODOs

## current TODOs

- [ ] for CGI child problem: there is these options:

    1) we are still writing and not everything has been written but receive
       EPOLLERR on the write fd. This means we could not write the whole reqBody
       to the pipe -> 504. What must be handled?

       - a 504 response can be send to the client immediately (no conn close!)
       - the child must be killed and/or reaped
       - if the client also disconnected (how to check that?)  -> disco client
       - disco client means: only after the child is completely gone remove the
         client from interest list and destroy

    2) we still writing and we receive EPOLLHUP on the read fd. this would be
       the same.

    3) we are done writing and receive a EPOLLHUP on the read fd. we should
       check child's exit status. if it exited with non-zero error or has not
       yet exited there is probably an error. if not, then we will have keep
       reading from the pipe until we get a read == 0. then we are fine and
       processing can continue as usual.

  So there is still to clarify how to handle the child killing. Should we
  continue processing the request while the child is not yet killed? meaning, we
  delay it until clients state is `CLI_CGIOK`? Or shall we do it asynchronously
  outside the main if-else-epoll-branch?

- [ ] add a cgi test where we write a line like "this is the start" and then a
  huge random bytes printable body and at the end a line "this is the end!" to
  show/check complete transmission.

- [ ] and this:

      A sender MUST NOT send whitespace between the start-line and the first
      header field.


- [ ] add func comments to hpp *not* to cpp files -> global visibility!



- [ ] find out what should be handled differently with other HTTP-versions

- [ ] add `isatty` based detection to `Logger` class in order to decide on
  colored output

- [ ] add more POST tests. especially one big file post test

- [ ] introduce `uploadPrefix` direc in route scope

- [ ] rename direcs like `upload` to `uploadDir` and `listen` -> `interface` and
  most importantly `errorPage` -> `statusPage`

- [ ] add more POST tests around the form-thingy also highlighting the Firefox
  413 / 400 on too large requests issue

- [ ] test virtual servers with 0.0.0.0 and more complex things

- [ ] test percent decoding in URIs

- [ ] add a `form-post-with-query.html` showcasing the query string based
  filename setting.

- [ ] what about routes with both CGI and upload?!?!?

- [ ] make maxbodysize number parsing reject too big numbers! what us u32MAX?
  along with that change the type to u64 as 4_295_967_295 bytes aka ~ 4.295 gigs
  is too small

- [ ] timeout script im CGI dashboard with custom error page

- [ ] make timeouts configurable via cfg

- [ ] make sure every cfg is terminated with `TOK_NULL`

- [ ] understand, fix, refactor CGI child process termination via epoll (child
  will send EPOLLHUP if kill was successful... but if not?)

- [ ] fix CGI timeout not happening.

- [x] add maximum body to CGI bodies

- [x] CGI and redir prohibited per cfg parsing

- [ ] rewrite RequestBody handling to just use a char array under the hood :(

- [ ] make Logger::logReqRes treat seperated body and hdrs params


## DONEs

- [x] fix hanging cfg:

- [x] finish virtual servers by making server listening on localhost:1111 and
      127.0.0.1:1111 with the same srvName distinguishable... **or let it be!**

- [x] impl redirection

- [x] GET THE CONFIG FORMAT STRAIGHT!!!

- [x] which config options will i support???

- [x] `GET /../bla.md HTTP/1.1` should not be working?!

- [x] implement tokenization

- [x] make the whole CNAME thing work. Problem is: that i work with the IP only
      from the very beginning.

- [x] according to [this
  sectio](https://datatracker.ietf.org/doc/html/rfc9112#name-field-syntax) all
  field-names are case-insensitive! So, i should convert them to lower and then
  the rest follows.

- [x] and this:

      In the interest of robustness, a server that is expecting to receive and
      parse a request-line SHOULD ignore at least one empty line (CRLF) received
      prior to the request-line.

- [x] **no two routes with same path are allowed in nginx!**
  + handled this by simply overwriting the same route with a potential new one

- [x] implement body / hdrs separation for requests that support bodies

- [x] implement Content-Length request handling. See [this
sec](https://datatracker.ietf.org/doc/html/rfc9112#name-message-parsing) for
instructions

- [x] implement forbidden methods handling. prereq: every route must have at
  least one allowed method! not allowed meths receive 403

- [x] impl proper routing in case of multiple routes (how does nginx do it?)

      For matching requests, the URI will be added to the path specified in the
      root directive, that is, to /data/www, to form the path to the requested
      file on the local file system. If there are several matching location
      blocks nginx selects the one with the **longest prefix**.

- [x] rename `Logger` funcs to camelCase!

- [x] if a GET or DELETE req is detected stop reading body from then on. but
  what if client is sending body?

- [x] Simple POST

- [x] implement autoindex!

- [x] if autoindex is not on and there is no index in a route which exists and is requested
via `/route/` then this is forbidden 403

- [x] support uploading file to the server using POST method, 

  + [x] either without query component (POST /upload HTTP/1.1) resulting in a
    canonical filename being chosen according to Content-Type, (POST /upload
    HTTP/1.1 \r\n Content-Type: image/jpeg -> /upload/file000.jpg being
    created), 

  + [x] or with query component (POST /upload?blabla.csv ... -> file is uploaded
    and stored as `upload-dir/blabla.csv`)

- [x] support URL redirection meaning: `redirect` directive will also take a
  full URL (like https://42.fr) as redir target!

- [x] check if a real-file-route behaves as expected. also with a redirect

- [x] add to URI class possibility to parse full URLs

- [x] CGI

- [x] DELETE

- [x] check if all getters return reasonable (const!?) values!

- [x] pay attention and implement sanitization according to this:

      A sender MUST NOT generate a bare CR (a CR character not immediately
      followed by LF) within any protocol elements other than the content. A
      recipient of such a bare CR MUST consider that element to be invalid or
      replace each bare CR with SP before processing the element or forwarding
      the message.

- [x] de-uglify the golbal var!
