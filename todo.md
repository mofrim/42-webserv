# webserv TODOs

## current TODOs

- [x] according to [this
  sectio](https://datatracker.ietf.org/doc/html/rfc9112#name-field-syntax) all
  field-names are case-insensitive! So, i should convert them to lower and then
  the rest follows.

- [ ] implement Content-Length request handling. See [this
  sec](https://datatracker.ietf.org/doc/html/rfc9112#name-message-parsing) for
  instructions

- [ ] pay attention and implement sanitization according to this:

      A sender MUST NOT generate a bare CR (a CR character not immediately
      followed by LF) within any protocol elements other than the content. A
      recipient of such a bare CR MUST consider that element to be invalid or
      replace each bare CR with SP before processing the element or forwarding
      the message.

- [x] and this:

      In the interest of robustness, a server that is expecting to receive and
      parse a request-line SHOULD ignore at least one empty line (CRLF) received
      prior to the request-line.

- [ ] and this:

      A sender MUST NOT send whitespace between the start-line and the first
      header field.

- [ ] add func comments to hpp *not* to cpp files -> global visibility!

- [ ] show errPage that is configured for a route / server

- [ ] check if all getters return reasonable (const!?) values!

- [x] **no two routes with same path are allowed in nginx!**
  + handled this by simply overwriting the same route with a potential new one

- [ ] impl proper routing in case of multiple routes (how does nginx do it?)

      For matching requests, the URI will be added to the path specified in the
      root directive, that is, to /data/www, to form the path to the requested
      file on the local file system. If there are several matching location
      blocks nginx selects the one with the **longest prefix**.

- [ ] impl redirection

- [ ] find out what should be handled differently with other HTTP-versions

- [ ] GET THE CONFIG FORMAT STRAIGHT!!!
  - [ ] which config options will i support???

- [ ] `GET /../bla.md HTTP/1.1` should not be working?!

- [ ] implement tokenization

- [ ] at least think about CGI

- [ ] support uploading file to the server using POST method, 
  + [ ] either without query component (POST /upload HTTP/1.1) resulting in a
    canonical filename being chosen according to Content-Type, (POST /upload
    HTTP/1.1 \r\n Content-Type: image/jpeg -> /upload/file000.jpg being
    created), 
  + [ ] or with query component (POST /upload?blabla.csv ... -> file is uploaded
    and stored as `upload-dir/blabla.csv`)

- [ ] add `isatty` based detection to `Logger` class in order to decide on
  colored output

## DONEs

- [x] fix hanging cfg:

- [x] finish virtual servers by making server listening on localhost:1111 and
      127.0.0.1:1111 with the same srvName distinguishable... **or let it be!**

## more on individual todos...

### clarifying the mutliple interface issue

- we are supposed to handle mutiple interfaces, meaning in the cfg it should be
  possible to specify something like

  ```nginx
  server {
    server_name bla;
    # -> will lead to listening on 10.0.0.1:80
    listen 10.0.0.1; 
    # standard
    listen localhost:8080;
    listen 127.0.0.1:9090;
    ...
  ```
  so, how are we gonna make this work? how do we treat all possible default
  values and stuff?
