// TODO: add singal handling for proper cleanup
// TODO: research if send/recv is preferred over read/write with sockets
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

int main(void)
{
	std::string response = "HTTP/1.1 200 OK\r\n\r\nhello from echo_server!\n";
	size_t			reslen	 = response.length();
	struct sockaddr_in addr;
	socklen_t					 addrlen = sizeof(addr);
	int								 srv_fd;

	srv_fd = socket(AF_INET, SOCK_STREAM, 0);

	int opt = 1;
	if (setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
					sizeof(opt)))
	{
		perror("setsockopt");
		return (1);
	}

	addr.sin_family			 = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port				 = htons(9090);

	if (bind(srv_fd, (struct sockaddr *)&addr, addrlen) == -1) {
		std::cout << "bind failed" << std::endl;
		return (1);
	}
	if (listen(srv_fd, 42) == -1) {
		std::cout << "listen failed" << std::endl;
		return (1);
	}

	int conncnt = 0;

	std::cout << ">>> echo_srv listening on 127.0.0.1:9090" << std::endl;
	while (conncnt < 10) {
		int	 client_fd = accept(srv_fd, (struct sockaddr *)&addr, &addrlen);
		char claddr[INET_ADDRSTRLEN];
		std::cout << ">>> got connection!\n";
		std::cout << ">>> from "
							<< inet_ntop(AF_INET, &addr.sin_addr, claddr, INET_ADDRSTRLEN)
							<< ":" << addr.sin_port << std::endl;

		char req[4096] = {0};
		if (read(client_fd, req, 4095) == -1) {
			std::cout << "read failed" << std::endl;
			return (-1);
		}
		std::cout << ">>> Received req:\n\n" << req << std::endl;
		if (write(client_fd, response.c_str(), reslen) == -1)
			std::cout << "write failed!" << std::endl;
		close(client_fd);
	}
	close(srv_fd);
}
