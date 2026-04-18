// this is a little playground script for experimenting with when bind fails

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

#define ADDR1 "0.0.0.0"
#define PORT1 "9999"

#define ADDR2 "10.0.0.1"
#define PORT2 "9998"

int main()
{
  struct addrinfo  hints;
  struct addrinfo *result, *rp;
  int              sfd0, s;
  char            *addrbuf = NULL;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_canonname = NULL;
  hints.ai_addr      = NULL;
  hints.ai_next      = NULL;
  hints.ai_family    = AF_INET;
  /* Allows IPv4 or IPv6 */
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE | AI_CANONNAME;
  s                 = getaddrinfo(ADDR1, PORT1, &hints, &result);
  if (s != 0) {
    std::cout << "getaddrinfo: " << gai_strerror(s) << std::endl;
    errno = ENOSYS;
    return -1;
  }

  /* Walk through returned list until we find an address structure
that can be used to successfully connect a socket */
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sfd0 = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd0 == -1) {
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
    std::cout << "trying to bind socket on '" << addrbuf << "'" << std::endl;
    std::cout << "rp->ai_addr: '"
              << ((struct sockaddr_in *)rp->ai_addr)->sin_addr.s_addr << "'\n"
              << "port: "
              << ntohs(((struct sockaddr_in *)rp->ai_addr)->sin_port)
              << std::endl;
    std::cout << "rp->ai_canonname: " << rp->ai_canonname << std::endl;
    if (bind(sfd0, rp->ai_addr, rp->ai_addrlen) != -1) {
      std::cout << "SUCCESS!" << std::endl;
      free(addrbuf);
      addrbuf = NULL;
      break; // Success!
    }
    /* Connect failed: close this socket and try next address */
    close(sfd0);
  }

  int sfd1;

  if (addrbuf != NULL)
    free(addrbuf);
  freeaddrinfo(result);

  s = getaddrinfo(ADDR2, PORT2, &hints, &result);
  if (s != 0) {
    std::cout << "getaddrinfo: " << gai_strerror(s) << std::endl;
    errno = ENOSYS;
    return -1;
  }

  /* Walk through returned list until we find an address structure that can be
   * used to successfully connect a socket */
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sfd1 = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd1 == -1) {
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
    std::cout << "trying to bind socket on '" << addrbuf << "'" << std::endl;
    std::cout << "rp->ai_addr: '"
              << ((struct sockaddr_in *)rp->ai_addr)->sin_addr.s_addr << "'\n"
              << "port: "
              << ntohs(((struct sockaddr_in *)rp->ai_addr)->sin_port)
              << std::endl;
    std::cout << "rp->ai_canonname: " << rp->ai_canonname << std::endl;
    if (bind(sfd1, rp->ai_addr, rp->ai_addrlen) != -1) {
      std::cout << "SUCCESS!" << std::endl;
      free(addrbuf);
      addrbuf = NULL;
      break; // Success!
    }
    else {
      perror("bind");
    }
    /* Connect failed: close this socket and try next address */
    close(sfd1);
  }
  if (addrbuf != NULL)
    free(addrbuf);
  freeaddrinfo(result);

  sleep(60);

  close(sfd0);
  close(sfd1);

  return (0);
}
