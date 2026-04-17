# werbserv Questions And Answers

## how to handle virtual servers and 127.0.0.1:80 vs. localhost:80?

It seems like nginx is opening only one socket for this. So...

**What happens if i bind a socket to each of the examples from title?**

- trying to bind to `localhost:9999` and `127.0.0.1:9999` fails

- binding `localhost:9999` and `10.0.0.1:9999` succeeds


