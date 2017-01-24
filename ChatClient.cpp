#include <iostream>
#include <algorithm>
#include <set>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <errno.h>
#include <string.h>

#define POLL_SIZE 32
#define MESSAGE_PART_LEN 1024

// MSG_NOSIGNAL does not exists on OS X
#if defined(__APPLE__) || defined(__MACH__)
# ifndef MSG_NOSIGNAL
#   define MSG_NOSIGNAL SO_NOSIGPIPE
# endif
#endif

int set_nonblock(int fd)
{
	int flags;
#if defined(O_NONBLOCK)
	if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
		flags = 0;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
	flags = 1;
	return ioctl(fd, FIOBIO, &flags);
#endif
}

void output(char* buffer, size_t len) {
	char stringBuffer[MESSAGE_PART_LEN + 1];
	memcpy(stringBuffer, buffer, len);
	stringBuffer[len] = '\0';
	std::cout << std::string(stringBuffer);
}

int main(int argc, char **argv)
{
	int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(MasterSocket == -1)
	{
		std::cout << strerror(errno) << std::endl;
		return 1;
	}

	struct sockaddr_in SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(3100);
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int Result = connect(MasterSocket, (struct sockaddr *)&SockAddr, sizeof(SockAddr)) < 0;
	if(Result == -1)
	{
		std::cout << strerror(errno) << std::endl;
		return 1;
	}

	set_nonblock(MasterSocket);
	set_nonblock(STDIN_FILENO);
	size_t SetSize = 2;
	struct pollfd Set[SetSize];
	Set[0].fd = STDIN_FILENO;
	Set[0].events = POLLIN;
	Set[1].fd = MasterSocket;
	Set[1].events = POLLIN;

	while(true)
	{
		poll(Set, SetSize, -1);
		for(unsigned int i = 0; i < SetSize; i++)
		{
			static char Buffer[MESSAGE_PART_LEN];
			if(Set[i].revents & POLLIN)
			{
				if(Set[i].fd == MasterSocket)
				{
					int RecvSize = read(MasterSocket, Buffer, MESSAGE_PART_LEN);					
					if((RecvSize == 0) && (errno != EAGAIN))
					{
						shutdown(Set[1].fd, SHUT_RDWR);
						close(Set[1].fd);
						std::cout << "Client disconnected by server" << std::endl;
						return 0;
					}
					else if(RecvSize != 0)
					{
						// Output string to STDOUT
						output(Buffer, RecvSize);
					}
				}
				else
				{   // Input from STDIN
					int len = 0;
					len = read(STDIN_FILENO, Buffer, MESSAGE_PART_LEN);
					write(MasterSocket, Buffer, len);
				}
			}
		}
	}
	return 0;
}
