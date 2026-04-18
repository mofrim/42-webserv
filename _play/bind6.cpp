#include <arpa/inet.h>
#include <cstdlib>
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <string>
#include <sys/socket.h>

int main()
{

	struct sockaddr_in6 addr;
	// socklen_t						adrlen = sizeof(addr);
	// int									sock;
	std::string ip("::1");
	char			 *ip_check = (char *)malloc(INET6_ADDRSTRLEN);

	// if ((sock = socket(AF_INET6, SOCK_STREAM, 0)) == -1) {
	// 	perror("socket");
	// 	exit(1);
	// }

	if (inet_pton(AF_INET6, ip.c_str(), &addr.sin6_addr) != 1) {
		perror("inet_pton");
		exit(1);
	}
	addr.sin6_port		 = htons(9999);
	addr.sin6_flowinfo = 0;
	addr.sin6_scope_id = 0;

	if (inet_ntop(AF_INET6, &addr.sin6_addr, ip_check, INET6_ADDRSTRLEN) == NULL)
	{
		perror("inet_ntop");
		exit(1);
	}
	std::cout << "ip.c_str(): " << ip.c_str() << std::endl;
	std::cout << "ip_check: " << ip_check << std::endl;

	return (0);
}
