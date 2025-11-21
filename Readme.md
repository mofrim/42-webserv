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

```cpp
#include "webserv.hpp"

WebservConfig cfg;
WebservParser parser;
Webserv webserv;

cfg = parser.parseCfg(av[1] ? av[1]: "path/to/default.conf");
if (cfg.fail()) {
 Logger::log.error(...);
 return (-42);
}
webserv.run(cfg);

return (0);

```
