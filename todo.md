# webserv TODOs

## current TODOs

- [ ] clarify how to initialize the interfaces for a server!!!
- [ ] implement multiple servers on the same port handling.
- [ ] implement tokenization
- [ ] at least think about CGI
- [ ] support uploading file to the server using POST method, 
  + [ ] either without query component (POST /upload HTTP/1.1) resulting in a
    canonical filename being chosen according to Content-Type, (POST /upload
    HTTP/1.1 \r\n Content-Type: image/jpeg -> /upload/file000.jpg being
    created), 
  + [ ] or with query component (POST /upload?blabla.csv ... -> file is uploaded
    and stored as `upload-dir/blabla.csv`)

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


