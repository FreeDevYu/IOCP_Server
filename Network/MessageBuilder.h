#pragma once
#include "Lock.h"
#include "NetworkDefine.h"

namespace Network
{
	class MessageBuilder : public DefaultLock
	{
	public:
		MessageBuilder();
		~MessageBuilder();

		int InsertMessage(void* message, int size);
		int MessageCheckAndReturn(char*& outBuffer, int& bufferSize);

	private:
		char* _buffer;
		int _bufferSize; // �� ���� �������� ũ��
	};
}