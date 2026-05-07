# Document / Tracker for possible testcases

- [ ] **Request**: `echo -en "GET /index.html? HTTP/1.1\r\nHost: moep\r\n\r\n" 1>&4 && cat <&4`
  + **Response**: serve page, no error

- [ ] **Request**: `echo -en "GET /index.html?miep=moep HTTP/1.1\r\nHost: moep\r\n\r\n" 1>&4 && cat <&4`
  + **Response**: serve page, no error

- [ ] **Request**: `echo -en "GET index.html HTTP/1.1\r\nHost: moep\r\n\r\n" 1>&4 && cat <&4`
  + **Response**: 400, requests target has to start with `/`

- [ ] **Request**: `echo -en "GET bla.org/index.html HTTP/1.1\r\nHost: moep\r\n\r\n" 1>&4 && cat <&4`
  + **Response**: 400, requests target has to start with `/`


