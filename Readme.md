# project ideas

## general ideas

- **use exceptions wherever possible!!!**

## structuring the project

### Partitioning

A Natural Partitioning would be:

1. webserver logic: sockets, polling, connection management =: **Pt1**
2. config file parsing and validation =: **Pt2**
3. http protocol implementation =: **Pt3**
4. CGI

### links between the partitions

#### how much of the protocol layer interacts with the webserver logic?

my guess right now is the following control flow:

- webserver launches listens on ports for incoming connections
- if a connection is established dispatches to **Pt3**
- what happens if connection breaks or anything goes wrong while **Pt3** is
  working?
- what to do with multiple connection on multiple servers?

## classes

1. `Webserv`
2. `Parser`
3. `Logger` but only as a callable class -> private constructors

## main.cpp

**how do i want this? what about the logger? simply copy the vsnprintf
approach?**

```cpp
#include "webserv.hpp"

WebservLogger logger;
WebservConfig cfg;
WebservParser parser;
Webserv webserv;

cfg = parser.parseCfg(av[1] ? av[1]: "path/to/default.conf", logger);
if (cfg.fail()) {
 logger.log_error(...);
 // logger.log_info(...);
 // logger.log_warn(...);
 return (-42);
}
webserv.run(cfg);

return (0);
```
## Client class, ConManager...

For now i will do all the connection handling from Webserv class. Why? Because
as i see by now, there won't be too much to do here and having an extra
ConnManager class **and** a Client class seems to much to me.
