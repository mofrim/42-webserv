
## the question

The man pages and programmer documentations for the socket options SO_REUSEADDR
and SO_REUSEPORT are different for different operating systems and often highly
confusing. Some operating systems don't even have the option SO_REUSEPORT. The
WWW is full of contradicting information regarding this subject and often you
can find information that is only true for one socket implementation of a
specific operating system, which may not even be explicitly mentioned in the
text.

So how exactly is SO_REUSEADDR different than SO_REUSEPORT?

Are systems without SO_REUSEPORT more limited?

And what exactly is the expected behavior if I use either one on different
operating systems?


## the answer

Welcome to the wonderful world of portability... or rather the lack of it.
Before we start analyzing these two options in detail and take a deeper look how
different operating systems handle them, it should be noted that the BSD socket
implementation is the mother of all socket implementations. Basically all other
systems copied the BSD socket implementation at some point in time (or at least
its interfaces) and then started evolving it on their own. Of course the BSD
socket implementation was evolved as well at the same time and thus systems that
copied it later got features that were lacking in systems that copied it
earlier. Understanding the BSD socket implementation is the key to understanding
all other socket implementations, so you should read about it even if you don't
care to ever write code for a BSD system.

There are a couple of basics you should know before we look at these two
options. A TCP/UDP connection is identified by a tuple of five values:

{<protocol>, <src addr>, <src port>, <dest addr>, <dest port>}

Any unique combination of these values identifies a connection. As a result, no
two connections can have the same five values, otherwise the system would not be
able to distinguish these connections any longer.

The protocol of a socket is set when a socket is created with the socket()
function. The source address and port are set with the bind() function. The
destination address and port are set with the connect() function. Since UDP is a
connectionless protocol, UDP sockets can be used without connecting them. Yet it
is allowed to connect them and in some cases very advantageous for your code and
general application design. In connectionless mode, UDP sockets that were not
explicitly bound when data is sent over them for the first time are usually
automatically bound by the system, as an unbound UDP socket cannot receive any
(reply) data. Same is true for an unbound TCP socket, it is automatically bound
before it will be connected.

If you explicitly bind a socket, it is possible to bind it to port 0, which
means "any port". Since a socket cannot really be bound to all existing ports,
the system will have to choose a specific port itself in that case (usually from
a predefined, OS specific range of source ports). A similar wildcard exists for
the source address, which can be "any address" (0.0.0.0 in case of IPv4 and ::
in case of IPv6). Unlike in case of ports, a socket can really be bound to "any
address" which means "all source IP addresses of all local interfaces". If the
socket is connected later on, the system has to choose a specific source IP
address, since a socket cannot be connected and at the same time be bound to any
local IP address. Depending on the destination address and the content of the
routing table, the system will pick an appropriate source address and replace
the "any" binding with a binding to the chosen source IP address.

By default, no two sockets can be bound to the same combination of source
address and source port. As long as the source port is different, the source
address is actually irrelevant. Binding socketA to ipA:portA and socketB to
ipB:portB is always possible if ipA != ipB holds true, even when portA == portB.
E.g. socketA belongs to a FTP server program and is bound to 192.168.0.1:21 and
socketB belongs to another FTP server program and is bound to 10.0.0.1:21, both
bindings will succeed. Keep in mind, though, that a socket may be locally bound
to "any address". If a socket is bound to 0.0.0.0:21, it is bound to all
existing local addresses at the same time and in that case no other socket can
be bound to port 21, regardless which specific IP address it tries to bind to,
as 0.0.0.0 conflicts with all existing local IP addresses.

Anything said so far is pretty much equal for all major operating system. Things
start to get OS specific when address reuse comes into play. We start with BSD,
since as I said above, it is the mother of all socket implementations.

### BSD SO_REUSEADDR

If SO_REUSEADDR is enabled on a socket prior to binding it, the socket can be
successfully bound unless there is a conflict with another socket bound to
exactly the same combination of source address and port. Now you may wonder how
is that any different than before? The keyword is "exactly". SO_REUSEADDR mainly
changes the way how wildcard addresses ("any IP address") are treated when
searching for conflicts.

Without SO_REUSEADDR, binding socketA to 0.0.0.0:21 and then binding socketB to
192.168.0.1:21 will fail (with error EADDRINUSE), since 0.0.0.0 means "any local
IP address", thus all local IP addresses are considered in use by this socket
and this includes 192.168.0.1, too. With SO_REUSEADDR it will succeed, since
0.0.0.0 and 192.168.0.1 are not exactly the same address, one is a wildcard for
all local addresses and the other one is a very specific local address. Note
that the statement above is true regardless in which order socketA and socketB
are bound; without SO_REUSEADDR it will always fail, with SO_REUSEADDR it will
always succeed.

To give you a better overview, let's make a table here and list all possible
combinations:

SO_REUSEADDR       socketA        socketB       Result
---------------------------------------------------------------------
  ON/OFF       192.168.0.1:21   192.168.0.1:21    Error (EADDRINUSE)
  ON/OFF       192.168.0.1:21      10.0.0.1:21    OK
  ON/OFF          10.0.0.1:21   192.168.0.1:21    OK
   OFF             0.0.0.0:21   192.168.1.0:21    Error (EADDRINUSE)
   OFF         192.168.1.0:21       0.0.0.0:21    Error (EADDRINUSE)
   ON              0.0.0.0:21   192.168.1.0:21    OK
   ON          192.168.1.0:21       0.0.0.0:21    OK
  ON/OFF           0.0.0.0:21       0.0.0.0:21    Error (EADDRINUSE)

The table above assumes that socketA has already been successfully bound to the
address given for socketA, then socketB is created, either gets SO_REUSEADDR set
or not, and finally is bound to the address given for socketB. Result is the
result of the bind operation for socketB. If the first column says ON/OFF, the
value of SO_REUSEADDR is irrelevant to the result.

Okay, SO_REUSEADDR has an effect on wildcard addresses, good to know. Yet that
isn't its only effect it has. There is another well known effect which is also
the reason why most people use SO_REUSEADDR in server programs in the first
place. For the other important use of this option we have to take a deeper look
on how the TCP protocol works.

If a TCP socket is being closed, normally a 3-way handshake is performed; the
sequence is called FIN-ACK. The problem here is, that the last ACK of that
sequence may have arrived on the other side or it may not have arrived and only
if it has, the other side also considers the socket as being fully closed. To
prevent re-using an address+port combination, that may still be considered open
by some remote peer, the system will not immediately consider a socket as dead
after sending the last ACK but instead put the socket into a state commonly
referred to as TIME_WAIT. It can be in that state for minutes (system dependent
setting). On most systems you can get around that state by enabling lingering
and setting a linger time of zero1 but there is no guarantee that this is always
possible, that the system will always honor this request, and even if the system
honors it, this causes the socket to be closed by a reset (RST), which is not
always a great idea. To learn more about linger time, have a look at my answer
about this topic.

The question is, how does the system treat a socket in state TIME_WAIT? If
SO_REUSEADDR is not set, a socket in state TIME_WAIT is considered to still be
bound to the source address and port and any attempt to bind a new socket to the
same address and port will fail until the socket has really been closed. So
don't expect that you can rebind the source address of a socket immediately
after closing it. In most cases this will fail. However, if SO_REUSEADDR is set
for the socket you are trying to bind, another socket bound to the same address
and port in state TIME_WAIT is simply ignored, after all its already "half
dead", and your socket can bind to exactly the same address without any problem.
In that case it plays no role that the other socket may have exactly the same
address and port. Note that binding a socket to exactly the same address and
port as a dying socket in TIME_WAIT state can have unexpected, and usually
undesired, side effects in case the other socket is still "at work", but that is
beyond the scope of this answer and fortunately those side effects are rather
rare in practice.

There is one final thing you should know about SO_REUSEADDR. Everything written
above will work as long as the socket you want to bind to has address reuse
enabled. It is not necessary that the other socket, the one which is already
bound or is in a TIME_WAIT state, also had this flag set when it was bound. The
code that decides if the bind will succeed or fail only inspects the
SO_REUSEADDR flag of the socket fed into the bind() call, for all other sockets
inspected, this flag is not even looked at.

### SO_REUSEPORT

SO_REUSEPORT is what most people would expect SO_REUSEADDR to be. Basically,
SO_REUSEPORT allows you to bind an arbitrary number of sockets to exactly the
same source address and port as long as all prior bound sockets also had
SO_REUSEPORT set before they were bound. If the first socket that is bound to an
address and port does not have SO_REUSEPORT set, no other socket can be bound to
exactly the same address and port, regardless if this other socket has
SO_REUSEPORT set or not, until the first socket releases its binding again.
Unlike in case of SO_REUSEADDR the code handling SO_REUSEPORT will not only
verify that the currently bound socket has SO_REUSEPORT set but it will also
verify that the socket with a conflicting address and port had SO_REUSEPORT set
when it was bound.

SO_REUSEPORT does not imply SO_REUSEADDR. This means if a socket did not have
SO_REUSEPORT set when it was bound and another socket has SO_REUSEPORT set when
it is bound to exactly the same address and port, the bind fails, which is
expected, but it also fails if the other socket is already dying and is in
TIME_WAIT state. To be able to bind a socket to the same addresses and port as
another socket in TIME_WAIT state requires either SO_REUSEADDR to be set on that
socket or SO_REUSEPORT must have been set on both sockets prior to binding them.
Of course it is allowed to set both, SO_REUSEPORT and SO_REUSEADDR, on a socket.

There is not much more to say about SO_REUSEPORT other than that it was added
later than SO_REUSEADDR, that's why you will not find it in many socket
implementations of other systems, which "forked" the BSD code before this option
was added, and that there was no way to bind two sockets to exactly the same
socket address in BSD prior to this option.
Connect() Returning EADDRINUSE?

Most people know that bind() may fail with the error EADDRINUSE, however, when
you start playing around with address reuse, you may run into the strange
situation that connect() fails with that error as well. How can this be? How can
a remote address, after all that's what connect adds to a socket, be already in
use? Connecting multiple sockets to exactly the same remote address has never
been a problem before, so what's going wrong here?

As I said on the very top of my reply, a connection is defined by a tuple of
five values, remember? And I also said, that these five values must be unique
otherwise the system cannot distinguish two connections any longer, right? Well,
with address reuse, you can bind two sockets of the same protocol to the same
source address and port. That means three of those five values are already the
same for these two sockets. If you now try to connect both of these sockets also
to the same destination address and port, you would create two connected
sockets, whose tuples are absolutely identical. This cannot work, at least not
for TCP connections (UDP connections are no real connections anyway). If data
arrived for either one of the two connections, the system could not tell which
connection the data belongs to. At least the destination address or destination
port must be different for either connection, so that the system has no problem
to identify to which connection incoming data belongs to.

So if you bind two sockets of the same protocol to the same source address and
port and try to connect them both to the same destination address and port,
connect() will actually fail with the error EADDRINUSE for the second socket you
try to connect, which means that a socket with an identical tuple of five values
is already connected.
Multicast Addresses

Most people ignore the fact that multicast addresses exist, but they do exist.
While unicast addresses are used for one-to-one communication, multicast
addresses are used for one-to-many communication. Most people got aware of
multicast addresses when they learned about IPv6 but multicast addresses also
existed in IPv4, even though this feature was never widely used on the public
Internet.

The meaning of SO_REUSEADDR changes for multicast addresses as it allows
multiple sockets to be bound to exactly the same combination of source multicast
address and port. In other words, for multicast addresses SO_REUSEADDR behaves
exactly as SO_REUSEPORT for unicast addresses. Actually, the code treats
SO_REUSEADDR and SO_REUSEPORT identically for multicast addresses, that means
you could say that SO_REUSEADDR implies SO_REUSEPORT for all multicast addresses
and the other way round.

### FreeBSD/OpenBSD/NetBSD

All these are rather late forks of the original BSD code, that's why they all
three offer the same options as BSD and they also behave the same way as in BSD.

### macOS (MacOS X)

At its core, macOS is simply a BSD-style UNIX named "Darwin", based on a rather
late fork of the BSD code (BSD 4.3), which was then later on even
re-synchronized with the (at that time current) FreeBSD 5 code base for the Mac
OS 10.3 release, so that Apple could gain full POSIX compliance (macOS is POSIX
certified). Despite having a microkernel at its core ("Mach"), the rest of the
kernel ("XNU") is basically just a BSD kernel, and that's why macOS offers the
same options as BSD and they also behave the same way as in BSD.
iOS / watchOS / tvOS

iOS is just a macOS fork with a slightly modified and trimmed kernel, somewhat
stripped down user space toolset and a slightly different default framework set.
watchOS and tvOS are iOS forks, that are stripped down even further (especially
watchOS). To my best knowledge they all behave exactly as macOS does.

### Linux Linux < 3.9

Prior to Linux 3.9, only the option SO_REUSEADDR existed. This option behaves
generally the same as in BSD with two important exceptions:

    As long as a listening (server) TCP socket is bound to a specific port, the
    SO_REUSEADDR option is entirely ignored for all sockets targeting that port.
    Binding a second socket to the same port is only possible if it was also
    possible in BSD without having SO_REUSEADDR set. E.g. you cannot bind to a
    wildcard address and then to a more specific one or the other way round,
    both is possible in BSD if you set SO_REUSEADDR. What you can do is you can
    bind to the same port and two different non-wildcard addresses, as that's
    always allowed. In this aspect Linux is more restrictive than BSD.

    The second exception is that for client sockets, this option behaves exactly
    like SO_REUSEPORT in BSD, as long as both had this flag set before they were
    bound. The reason for allowing that was simply that it is important to be
    able to bind multiple sockets to exactly to the same UDP socket address for
    various protocols and as there used to be no SO_REUSEPORT prior to 3.9, the
    behavior of SO_REUSEADDR was altered accordingly to fill that gap. In that
    aspect Linux is less restrictive than BSD.

### Linux >= 3.9

Linux 3.9 added the option SO_REUSEPORT to Linux as well. This option behaves
exactly like the option in BSD and allows binding to exactly the same address
and port number as long as all sockets have this option set prior to binding
them.

Yet, there are still two differences to SO_REUSEPORT on other systems:

    To prevent "port hijacking", there is one special limitation: All sockets
    that want to share the same address and port combination must belong to
    processes that share the same effective user ID! So one user cannot "steal"
    ports of another user. This is some special magic to somewhat compensate for
    the missing SO_EXCLBIND/SO_EXCLUSIVEADDRUSE flags.

    Additionally the kernel performs some "special magic" for SO_REUSEPORT
    sockets that isn't found in other operating systems: For UDP sockets, it
    tries to distribute datagrams evenly, for TCP listening sockets, it tries to
    distribute incoming connect requests (those accepted by calling accept())
    evenly across all the sockets that share the same address and port
    combination. Thus an application can easily open the same port in multiple
    child processes and then use SO_REUSEPORT to get a very inexpensive load
    balancing.


### Android

Even though the whole Android system is somewhat different from most Linux
distributions, at its core works a slightly modified Linux kernel, thus
everything that applies to Linux should apply to Android as well.

### Windows

Windows only knows the SO_REUSEADDR option, there is no SO_REUSEPORT. Setting
SO_REUSEADDR on a socket in Windows behaves like setting SO_REUSEPORT and
SO_REUSEADDR on a socket in BSD, with one exception:

Prior to Windows 2003, a socket with SO_REUSEADDR could always been bound to
exactly the same source address and port as an already bound socket, even if the
other socket did not have this option set when it was bound. This behavior
allowed an application "to steal" the connected port of another application.
Needless to say that this has major security implications!

Microsoft realized that and added another important socket option:
SO_EXCLUSIVEADDRUSE. Setting SO_EXCLUSIVEADDRUSE on a socket makes sure that if
the binding succeeds, the combination of source address and port is owned
exclusively by this socket and no other socket can bind to them, not even if it
has SO_REUSEADDR set.

This default behavior was changed first in Windows 2003, Microsoft calls that
"Enhanced Socket Security" (funny name for a behavior that is default on all
other major operating systems). For more details just visit this page. There are
three tables: The first one shows the classic behavior (still in use when using
compatibility modes!), the second one shows the behavior of Windows 2003 and up
when the bind() calls are made by the same user, and the third one when the
bind() calls are made by different users.

### Solaris

Solaris is the successor of SunOS. SunOS was originally based on a fork of BSD,
SunOS 5 and later was based on a fork of SVR4, however SVR4 is a merge of BSD,
System V, and Xenix, so up to some degree Solaris is also a BSD fork, and a
rather early one. As a result Solaris only knows SO_REUSEADDR, there is no
SO_REUSEPORT. The SO_REUSEADDR behaves pretty much the same as it does in BSD.
As far as I know there is no way to get the same behavior as SO_REUSEPORT in
Solaris, that means it is not possible to bind two sockets to exactly the same
address and port.

Similar to Windows, Solaris has an option to give a socket an exclusive binding.
This option is named SO_EXCLBIND. If this option is set on a socket prior to
binding it, setting SO_REUSEADDR on another socket has no effect if the two
sockets are tested for an address conflict. E.g. if socketA is bound to a
wildcard address and socketB has SO_REUSEADDR enabled and is bound to a
non-wildcard address and the same port as socketA, this bind will normally
succeed, unless socketA had SO_EXCLBIND enabled, in which case it will fail
regardless the SO_REUSEADDR flag of socketB.

### Other Systems

In case your system is not listed above, I wrote a little test program that you
can use to find out how your system handles these two options. Also if you think
my results are wrong, please first run that program before posting any comments
and possibly making false claims.

All that the code requires to build is a bit POSIX API (for the network parts)
and a C99 compiler (actually most non-C99 compiler will work as well as long as
they offer inttypes.h and stdbool.h; e.g. gcc supported both long before
offering full C99 support).

All that the program needs to run is that at least one interface in your system
(other than the local interface) has an IP address assigned and that a default
route is set which uses that interface. The program will gather that IP address
and use it as the second "specific address".

It tests all possible combinations you can think of:

    TCP and UDP protocol
    Normal sockets, listen (server) sockets, multicast sockets
    SO_REUSEADDR set on socket1, socket2, or both sockets
    SO_REUSEPORT set on socket1, socket2, or both sockets
    All address combinations you can make out of 0.0.0.0 (wildcard), 127.0.0.1 (specific address), and the second specific address found at your primary interface (for multicast it's just 224.1.2.3 in all tests)

and prints the results in a nice table. It will also work on systems that don't
know SO_REUSEPORT, in which case this option is simply not tested.

What the program cannot easily test is how SO_REUSEADDR acts on sockets in
TIME_WAIT state as it's very tricky to force and keep a socket in that state.
Fortunately most operating systems seems to simply behave like BSD here and most
of the time programmers can simply ignore the existence of that state.

Here's the code (I cannot include it here, answers have a size limit and the
code would push this reply over the limit).

### the sourcecode

```c
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <netinet/in.h>

#if defined(__APPLE__) || defined(__bsd__)
#	define HAVE_SA_LEN
#endif

static const uint16_t kTestPort = 23999;

typedef enum {
	SocModeNone = 0,
	SocModeListen,
	SocModeMulticast
} SocketMode;

typedef enum {
	SocReNone = 0,
	SocReAddr1,
	SocReAddr2,
	SocReAddrBoth,
#ifndef SO_REUSEPORT
	SocReLast,
#endif
	SocRePort1,
	SocRePort2,
	SocRePortBoth
#ifdef SO_REUSEPORT
	,
	SocReLast
#endif
} SocketReuse;

static int socketA = -1;
static int socketB = -1;

static
void cleanUp ( ) {
	if (socketA >= 0) close(socketA);
	if (socketB >= 0) close(socketB);
	socketA = -1;
	socketB = -1;
}

static
bool setupUDP ( ) {
	socketA = socket(PF_INET, SOCK_DGRAM, 0);
	socketB = socket(PF_INET, SOCK_DGRAM, 0);
	if (socketA < 0 || socketB < 0) cleanUp();
	return (socketA >= 0);
}

static
bool setupTCP ( ) {
	socketA = socket(PF_INET, SOCK_STREAM, 0);
	socketB = socket(PF_INET, SOCK_STREAM, 0);
	if (socketA < 0 || socketB < 0) cleanUp();
	return (socketA >= 0);
}

static
bool enableSockOpt ( int socket, int option, bool enable ) {
	int yes = (enable ? 1 : 0);
	int err = setsockopt(socket, SOL_SOCKET, option,  &yes, sizeof(yes));
	return !err;
}

static
bool enableReuseAddr ( int socket, bool enable ) {
	return enableSockOpt(socket, SO_REUSEADDR, enable);
}

static
bool enableReusePort ( int socket, bool enable ) {
#ifdef SO_REUSEPORT
	return enableSockOpt(socket, SO_REUSEPORT, enable);
#else
	return true;
#endif
}

static
struct sockaddr_in makeSockaddr (
	const char * localAddr, uint16_t localPort
) {
	struct sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
#ifdef HAVE_SA_LEN
	addr.sin_len = sizeof(addr);
#endif
	addr.sin_port = htons((unsigned short)localPort);
	int ok = inet_pton(AF_INET, localAddr, &addr.sin_addr);
	if (!ok) addr.sin_family = AF_UNSPEC;
	return addr;
}

static
bool bindSocket ( int socket, const char * localAddr, uint16_t localPort ) {
	struct sockaddr_in addr = makeSockaddr(localAddr, localPort);
	if (addr.sin_family == AF_UNSPEC) return false;

	int err = bind(socket, (struct sockaddr *)&addr, sizeof(addr));
	return !err;
}

static
bool makeListenSocket ( int socket ) {
	int err = listen(socket, 1);
	return !err;
}


static
bool test (
	SocketMode mode, bool useTCP, SocketReuse reuse,
	const char * localAddress1, const char * localAddress2
) {
	if (useTCP) {
		// TCP cannot be multicast!
		if (mode == SocModeMulticast) { errno = EINVAL; return false; }
		if (!setupTCP()) return false;
	} else {
		// UDP cannot be listen!
		if (mode == SocModeListen) { errno = EINVAL; return false; }
		if (!setupUDP()) return false;
	}

	bool reuseAddr1 = (reuse == SocReAddr1 || reuse == SocReAddrBoth);
	bool reuseAddr2 = (reuse == SocReAddr2 || reuse == SocReAddrBoth);
	bool reusePort1 = (reuse == SocRePort1 || reuse == SocRePortBoth);
	bool reusePort2 = (reuse == SocRePort2 || reuse == SocRePortBoth);

	if (!enableReuseAddr(socketA, reuseAddr1)
		|| !enableReuseAddr(socketB, reuseAddr2)
		|| !enableReusePort(socketA, reusePort1)
		|| !enableReusePort(socketB, reusePort2)
	) {
		cleanUp();
		return false;
	}

	if (!bindSocket(socketA, localAddress1, kTestPort)) {
		cleanUp();
		return false;
	}

	if (mode == SocModeListen) {
		if (!makeListenSocket(socketA)) { cleanUp(); return false; }
	}

	char * modeName = NULL;
	switch (mode) {
		case SocModeNone:      modeName = "(none)   "; break;
		case SocModeListen:    modeName = "Listen   "; break;
		case SocModeMulticast: modeName = "Multicast"; break;
		default: cleanUp(); errno = EINVAL; return false;
	}

	char * reuseName = NULL;
	switch (reuse) {
		case SocReNone:     reuseName = "(none)   "; break;
		case SocReAddr1:    reuseName = "Addr(1)  "; break;
		case SocReAddr2:    reuseName = "Addr(2)  "; break;
		case SocReAddrBoth: reuseName = "Addr(1&2)"; break;
		case SocRePort1:    reuseName = "Port(1)  "; break;
		case SocRePort2:    reuseName = "Port(2)  "; break;
		case SocRePortBoth: reuseName = "Port(1&2)"; break;
		case SocReLast: cleanUp(); errno = EINVAL; return false;
	}

	// INET_ADDRSTRLEN includes terminating \0 in count!
	int padding1 = (int)(INET_ADDRSTRLEN - strlen(localAddress1) - 1);
	int padding2 = (int)(INET_ADDRSTRLEN - strlen(localAddress2) - 1);

	int err = bindSocket(socketB, localAddress2, kTestPort);
	int errNo = (err ? 0 : errno);

	printf(
		"%s  "
		"%s    "
		"%s  "
		"%s%.*s  "
		"%s%.*s  "
		"->  %s%s%s%s\n",
		modeName,
		useTCP  ? "TCP" : "UDP",
		reuseName,
		localAddress1, padding1, "                ",
		localAddress2, padding2, "                ",
		errNo == 0 ? "OK" : "Error!",
		errNo == 0 ? "" : " (",
		errNo == 0 ? "" : strerror(errno),
		errNo == 0 ? "" : ")"
	);
	cleanUp();
	return true;
}

static
void testAndFailOnCriticalError (
	SocketMode mode, bool useTCP, SocketReuse reuse,
	const char * localAddress1, const char * localAddress2
) {
	bool ok = test(
		mode, useTCP, reuse,
		localAddress1, localAddress2
	);
	if (!ok) {
		fprintf(stderr, "Critical error setting up test! (%s)\n",
			strerror(errno)
		);
		exit(EXIT_FAILURE);
	}
}


static
char * copyPrimaryAddress ( ) {
	int so = socket(PF_INET, SOCK_DGRAM, 0);
	if (so < 0) return NULL;

	struct sockaddr_in addr = makeSockaddr("8.8.8.8", 443);
	if (addr.sin_family == AF_UNSPEC) { close(so); return false; }

	int err = connect(so, (struct sockaddr *)&addr, sizeof(addr));
	if (err) { close(so); return NULL; }

	socklen_t len = sizeof(addr);
	err = getsockname(so, (struct sockaddr *)&addr, &len);
	close(so);
	if (err) return NULL;

	// INET_ADDRSTRLEN includes terminating \0 in count!
	char buffer[INET_ADDRSTRLEN] = { 0 };
	const char * res = inet_ntop(
		AF_INET, &addr.sin_addr, buffer, sizeof(buffer)
	);
	if (!res) return NULL;

	// Apparently `strdup()` is not as potrable as one might expect
	size_t resultLength = strlen(res) + 1;
	char * result = malloc(resultLength);
	if (!result) return NULL;

	memcpy(result, res, resultLength);
	return result;
}


int main (
	int argc, const char * argv[]
) {
	const char *const localAddress = "127.0.0.1";
	const char *const wildcardAddress = "0.0.0.0";
	const char *const multicastAddress = "224.1.2.3";

#ifndef SO_REUSEPORT
	printf("WARNING: SO_REUESPORT is not available! "
		"Tests requiring it will just be skipped.\n"
	);
#endif

	printf("Test port is %"PRIu16"...\n", kTestPort);

	char * primaryAddress = copyPrimaryAddress();
	if (!primaryAddress) {
		fprintf(stderr, "Cannot obtain primary interface address!\n");
		return EXIT_FAILURE;
	}
	printf("Primary address: %s...\n", primaryAddress);

	if (strcmp(primaryAddress, localAddress) == 0) {
		fprintf(stderr, "Local address must not be primary address!");
		return EXIT_FAILURE;
	}

	const char *const sourceAddresses[] = {
		wildcardAddress, localAddress, primaryAddress
	};
	size_t addressCount = sizeof(sourceAddresses) / sizeof(sourceAddresses[0]);

	printf(
		"MODE       PROTO  REUSE      "
		"ADDRESS1         ADDRESS2         -> RESULT\n"
	);

	// Test every combinations but multicast
	SocketMode mode;
	for (mode = SocModeNone; mode < SocModeMulticast; mode++) {

		for (int proto = 0; proto < 2; proto++) {
			bool useTCP = (proto == 0);
			// UDP cannot be listen
			if (!useTCP && mode == SocModeListen) continue;

			SocketReuse reuse;
			for (reuse = SocReNone; reuse < SocReLast; reuse++) {

				size_t addr1;
				for (addr1 = 0; addr1 < addressCount; addr1++) {

					size_t addr2;
					for (addr2 = 0; addr2 < addressCount; addr2++) {

						// UDP cannot be listen!
						if (!useTCP && mode == SocModeListen) continue;

						testAndFailOnCriticalError(
							mode, useTCP, reuse,
							sourceAddresses[addr1], sourceAddresses[addr2]
						);
					}
				}
			}
		}
	}

	// Test all multicast combinations
	SocketReuse reuse;
	for (reuse = SocReNone; reuse < SocReLast; reuse++) {

		size_t addr1;
		for (addr1 = 0; addr1 < addressCount; addr1++) {

			size_t addr2;
			for (addr2 = 0; addr2 < addressCount; addr2++) {

				testAndFailOnCriticalError(
					SocModeMulticast, false, reuse,
					multicastAddress, multicastAddress
				);
			}
		}
	}

	return EXIT_SUCCESS;
}

```
