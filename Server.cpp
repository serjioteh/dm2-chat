#include "Server.h"
#include "Client.h"

const char* GREETING = "Welcome to chat!\n";
const size_t GREEINTG_LEN = 17;

/*
 * bind(2) and listen(2) to a tcp port
 */
	int
Server::mksock(const char *addr, int port)
{
	int i, sock;
	struct sockaddr_in serv;

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == -1)
	{
		std::cout << strerror(errno) << std::endl;
		return 1;
	}

	i = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&i,
				(socklen_t)sizeof(i)) == -1)
		warn("setsockopt");

	memset(&serv, 0, sizeof(struct sockaddr_in));
	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);
	serv.sin_addr.s_addr = inet_addr(addr);

	i = bind(sock, (struct sockaddr *)&serv, (socklen_t)sizeof(serv));
	if (i == -1)
	{
		std::cout << strerror(errno) << std::endl;
		return 1;
	}

	set_nonblock(sock);	

	i = listen(sock, SOMAXCONN);
	if (i == -1)
	{
		std::cout << strerror(errno) << std::endl;
		return 1;
	}

	return(sock);
}


Server::Server(int port) :
	m_portNumber { port }
	{
		// TODO Auto-generated constructor stub
	}

int Server::start() {
	/* get a listening socket */
	const char *addr = "127.0.0.1";
	servsock = mksock(addr, m_portNumber);

	/* get our kqueue descriptor */
	kq = kqueue();
	if (kq == -1)
	{
		std::cout << strerror(errno) << std::endl;
		return 1;
	}

	memset(&ke, 0, sizeof(struct kevent));

	/* fill out the kevent struct */
	EV_SET(&ke, servsock, EVFILT_READ, EV_ADD, 0, 5, NULL);

	/* set the event */
	i = kevent(kq, &ke, 1, NULL, 0, NULL);
	if (i == -1)
	{
		std::cout << strerror(errno) << std::endl;
		return 1;
	}

	while(true)
	{
		memset(&ke, 0, sizeof(ke));
		
		/* receive an event, a blocking call as timeout is NULL */
		i = kevent(kq, NULL, 0, &ke, 1, NULL);
		if (i == -1)
		{
			std::cout << strerror(errno) << std::endl;
			return 1;
		}

		if (i == 0)
			continue;

		/*
		 * since we only have one kevent in the eventlist, we're only
		 * going to get one event at a time
		 */

		if (ke.ident == servsock) 
		{
			/* server socket, theres a client to accept */
			len = (socklen_t)sizeof(c);
			int SlaveSocket = accept(servsock, (struct sockaddr *)&c, &len);
//			int SlaveSocket = accept(servsock, 0, 0);
		
			if (SlaveSocket == -1)
			{
				std::cout << strerror(errno) << std::endl;
				return 1;
			}

			set_nonblock(SlaveSocket);
			Client* client_ptr = new Client(SlaveSocket, this);
			m_clients[SlaveSocket] = std::unique_ptr<Client>(client_ptr);
			send(SlaveSocket, GREETING, GREEINTG_LEN, MSG_NOSIGNAL);

			memset(&ke, 0, sizeof(ke));
			EV_SET(&ke, SlaveSocket, EVFILT_READ, EV_ADD, 0, 0, NULL);
			i = kevent(kq, &ke, 1, NULL, 0, NULL);
			if (i == -1)
			{
				std::cout << strerror(errno) << std::endl;
				return 1;
			}		
			std::cout << "accepted connection" << std::endl;
		} 
		else 
		{
			/*
			 * got a message to distribute, first find the user
			 * and read their message
			 */

			Client* cl_ptr = m_clients[ke.ident].get();
			static char Buffer[MESSAGE_PART_LEN];
			memset(Buffer, 0, sizeof(Buffer));
			//int RecvSize = recv(ke.ident, Buffer, 1024, MSG_NOSIGNAL);
			int RecvSize = read(ke.ident, Buffer, MESSAGE_PART_LEN);
			
			if (RecvSize <= 0) 
			{ /* EOF from a client */
				EV_SET(&ke, ke.ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);
				i = kevent(kq, &ke, 1, 0, 0, NULL);
				if (i == -1)
				{
					std::cout << strerror(errno) << std::endl;
					return 1;
				}
				close(ke.ident);
				m_clients.erase(ke.ident);
				std::cout << "connection terminated" << std::endl;
			}
			else 
			{
				cl_ptr->schedule_message_cast(Buffer, RecvSize);
				//send(ke.ident, Buffer, RecvSize, MSG_NOSIGNAL);
				//write(ke.ident, Buffer, RecvSize);			
			}
		} /* end if */
	} /* end while */

return 0;
}

int Server::set_nonblock(int fd)
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

void Server::broadcast(const char* pData, size_t len) {
	for (auto client : m_clients) {
		int fd = client.first;
		send(fd, pData, len, MSG_NOSIGNAL);
	}
	char stringBuffer[MESSAGE_PART_LEN + 1];
	memcpy(stringBuffer, pData, len);
	stringBuffer[len] = '\0';
	std::cout << std::string(stringBuffer) << std::flush;
}
