# webserv TODOs

## current TODOs

- [ ] fix hanging cfg:

  ```cpp
  void Webserv::_initDefaultCfg()
  {
    VServerCfg cfg1;
    Route      r;
    cfg1.setServerName("virtual1");
    cfg1.addInterface("127.0.0.1", 1111);
    cfg1.addRoute(r);
    VServer dsrv1(cfg1);

    VServerCfg cfg2;
    cfg2.setServerName("virtual1");
    cfg2.addInterface("localhost", 1111);
    cfg2.addRoute(r);
    VServer dsrv2(cfg2);

    _vservers.push_back(dsrv1);
    _vservers.push_back(dsrv2);
  }
  ```

- [ ] GET THE CONFIG FORMAT STRAIGHT!!!
  - [ ] which config options will i support???
- [ ] `GET /../bla.md HTTP/1.1` should not be working?!
- [ ] finish virtual servers by making server listening on localhost:1111 and
      127.0.0.1:1111 with the same srvName distinguishable... **or let it be!**
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
