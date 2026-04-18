#include <arpa/inet.h>
#include <cstdlib>
#include <errno.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main()
{
  struct addrinfo  hints;
  struct addrinfo *result, *rp;
  int              sfd, s;
  char            *addrbuf = NULL;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_canonname = NULL;
  hints.ai_addr      = NULL;
  hints.ai_next      = NULL;
  hints.ai_family    = AF_INET;
  /* Allows IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM;
  // hints.ai_flags    = AI_PASSIVE | AI_NUMERICSERV;
  // hints.ai_flags = AI_CANONNAME;
  hints.ai_flags = AI_PASSIVE | AI_CANONNAME | AI_NUMERICSERV;

  s = getaddrinfo("10.0.0.1", "22", &hints, &result);
  if (s != 0) {
    std::cout << "getaddrinfo: " << gai_strerror(s) << std::endl;
    errno = ENOSYS;
    return -1;
  }

  /* Walk through returned list until we find an address structure that can be
   * used to successfully connect a socket */

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    addrbuf = (char *)malloc(rp->ai_addrlen);
    memset(addrbuf, 0, rp->ai_addrlen);

    switch (rp->ai_family) {
    case AF_INET:
      std::cout << "AF_INET" << std::endl;
      break;
    case AF_INET6:
      std::cout << "AF_INET6" << std::endl;
      break;
    default:
      std::cout << "Unknown family" << std::endl;
    }

    if (rp->ai_family == AF_INET || rp->ai_family == AF_INET6) {
      if (inet_ntop(rp->ai_family,
              &((struct sockaddr_in *)rp->ai_addr)->sin_addr,
              addrbuf,
              rp->ai_addrlen) == NULL)
      {
        perror("inet_ntop");
        free(addrbuf);
        exit(1);
      }
    }

    std::cout << "trying to bind socket on '" << addrbuf << "'" << std::endl;
    std::cout << "rp->ai_addr: '"
              << ((struct sockaddr_in *)rp->ai_addr)->sin_addr.s_addr << "'\n"
              << "port: "
              << ntohs(((struct sockaddr_in *)rp->ai_addr)->sin_port)
              << std::endl;
    std::cout << "rp->ai_canonname: " << rp->ai_canonname << std::endl;
    if (bind(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
      perror("bind");
      std::cout << "SUCCESS!" << std::endl;
      free(addrbuf);
      addrbuf = NULL;
      break; // Success!
    }
    /* Connect failed: close this socket and try next address */
    close(sfd);
  }
  if (addrbuf != NULL)
    free(addrbuf);
  freeaddrinfo(result);

  return (0);
}
