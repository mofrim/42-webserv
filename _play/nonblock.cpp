#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

#define BUFSIZE 1024

int main()
{
	int	 pipefds[2];
	char buf[1024];
	int	 flags;

	if (pipe(pipefds) == -1) {
		std::cout << "failed to open pipe" << std::endl;
		return (-1);
	}
	flags = fcntl(pipefds[0], F_GETFL);
	flags |= O_NONBLOCK;
	fcntl(pipefds[0], F_SETFL, flags);

	int bytes_read = read(pipefds[0], buf, BUFSIZE);
	std::cout << "errno: " << errno << std::endl;

	std::cout << "bytes_read: " << bytes_read << std::endl;

	return (0);
}
