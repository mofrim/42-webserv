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

#define HOST "localhost"

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
  hints.ai_flags    = AI_PASSIVE | AI_NUMERICSERV;
  s                 = getaddrinfo(HOST, "1234", &hints, &result);
  if (s != 0) {
    std::cout << "getaddrinfo: " << gai_strerror(s) << std::endl;
    errno = ENOSYS;
    return -1;
  }

  /* Walk through returned list until we find an address structure
that can be used to successfully connect a socket */
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd == -1) {
      perror("socket()");
      continue; // on error try next address
    }
    addrbuf = (char *)malloc(rp->ai_addrlen);
    memset(addrbuf, 0, rp->ai_addrlen);
    if (rp->ai_family == AF_INET) {
      std::cout << "AF_INET" << std::endl;
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
    std::cout << "trying to bind socket on '" << HOST << "'" << std::endl;
    std::cout
        << "rp->ai_addr: '"
        // << inet_ntop(((struct sockaddr_in *)rp->ai_addr)->sin_addr.s_addr)
        << addrbuf << "'\n"
        << "port: " << ntohs(((struct sockaddr_in *)rp->ai_addr)->sin_port)
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
