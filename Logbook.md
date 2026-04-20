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
