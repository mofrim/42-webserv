// this client has the only purpose of provoking a blocking I/O on our webserv

#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 4284

int main()
{
	int								 sock;
	struct sockaddr_in srv_addr;
	socklen_t					 addrlen;
	struct in_addr		 ipaddr;

	sock = socket(AF_INET, SOCK_STREAM, 0);

	srv_addr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &ipaddr);
	srv_addr.sin_addr = ipaddr;
	srv_addr.sin_port = htons(PORT);

	addrlen = sizeof(srv_addr);
	if (connect(sock, (struct sockaddr *)&srv_addr, addrlen) == -1) {
		std::cout << "connect failed" << std::endl;
		std::cout << "errno: " << errno << std::endl;
		close(sock);
		return (1);
	}

	write(sock, "a", 1);
	sleep(4);
	write(sock, "b", 1);
	sleep(4);
	write(sock, "c", 1);
	sleep(4);
	write(sock, "d", 1);

	close(sock);
}
