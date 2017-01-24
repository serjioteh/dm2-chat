#ifndef SERVER_H_
#define SERVER_H_

#pragma once
#include "Constants.h"

class Client;

class Server {
public:
	Server(int port);
	int start();
	void broadcast(const char* pData, size_t len);
private:
	int servsock, kq, i;
	struct kevent ke;
	struct sockaddr_in c; /* client */
	socklen_t len;
	
	int m_portNumber;
	std::map<int, std::shared_ptr<Client>> m_clients;
	static int set_nonblock(int fd);
	int mksock(const char *addr, int port);
};

#endif /* SERVER_H_ */
