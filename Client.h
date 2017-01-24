#ifndef CLIENT_H_
#define CLIENT_H_

#pragma once

//class Server;

#include "Constants.h"
#include "Server.h"

class Client {
public:
	Client();
	Client(int fd, Server* owner);
	void schedule_message_cast(const char* pData, size_t len);
private:
	std::unique_ptr<char> m_messageBuffer;
	int m_bufferBytesUsed;
	Server* m_owner;
	void cast_message();
};

#endif /* CLIENT_H_ */
