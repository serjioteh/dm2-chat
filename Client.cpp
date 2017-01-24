#include "Client.h"

Client::Client():
	m_bufferBytesUsed { 0 },
	m_owner { nullptr }
{
}

Client::Client(int fd, Server* owner):	
	m_bufferBytesUsed { 0 },
	m_owner { owner }
{
	char* pBuff = new char[MESSAGE_PART_LEN];
	m_messageBuffer = std::unique_ptr<char>(pBuff);
}

void Client::schedule_message_cast(const char* pData, size_t len) {
	size_t bytesFree = MESSAGE_PART_LEN - m_bufferBytesUsed;
	if (bytesFree >= len) {
		memcpy(m_messageBuffer.get() + m_bufferBytesUsed, pData, len);
		m_bufferBytesUsed += len;
		cast_message();
	}
	else {
		size_t rest = len - bytesFree;
		memcpy(m_messageBuffer.get() + m_bufferBytesUsed, pData, bytesFree);
		m_bufferBytesUsed = MESSAGE_PART_LEN;
		cast_message();
		memcpy(m_messageBuffer.get() + m_bufferBytesUsed, pData, rest);
		m_bufferBytesUsed = rest;
	}
}

void Client::cast_message() 
{
	while (m_bufferBytesUsed > 0) 
	{
		int delimiterPos = -1;
		for (int pos = 0; pos < m_bufferBytesUsed; pos++) 
		{
			if (m_messageBuffer.get()[pos] == '\n') 
			{
				delimiterPos = pos;
				break;
			}
		}
		if (delimiterPos == -1) 
		{
			if (m_bufferBytesUsed == MESSAGE_PART_LEN) 
			{
				m_owner->broadcast(m_messageBuffer.get(), MESSAGE_PART_LEN);
			}
			else 
			{
				break;
			}
		}
		else 
		{
			m_owner->broadcast(m_messageBuffer.get(), size_t(delimiterPos + 1));
			memcpy(m_messageBuffer.get(),
					m_messageBuffer.get() + delimiterPos + 1,
					m_bufferBytesUsed - delimiterPos - 1);
			m_bufferBytesUsed -= delimiterPos + 1;
		}
	}
}
