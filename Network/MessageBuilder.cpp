#include "MessageBuilder.h"

namespace Network
{

	MessageBuilder::MessageBuilder():	_bufferSize(0)
	{
		_buffer = new char[NET_DATA_BUFSIZE];
		memset(_buffer, 0, NET_DATA_BUFSIZE);
	}


	MessageBuilder::~MessageBuilder()
	{

	}

	int MessageBuilder::InsertMessage(void* message, int size)
	{
		if (message == NULL)
		{
			return NETWORK_ERROR; 
		}

		if (size <= 0 || size > NET_DATA_BUFSIZE)
		{
			return NETWORK_ERROR;
		}

		LockOn();

		memcpy(_buffer + _bufferSize, message, size);
		_bufferSize += size;

		LockOff();

		return NETWORK_OK;
	}

	int MessageBuilder::MessageCheckAndReturn(char* outBuffer, int bufferSize)
	{
		int headerSize = sizeof(MessageHeader);
	
		if(_bufferSize < headerSize)
		{
			return NETWORK_ERROR; // ���ۿ� �޽����� ����.
		}

		MessageHeader header{ 0,0,0 };
		std::memcpy(&header, _buffer, sizeof(MessageHeader));

		if(header.BodySize <= 0)
		{
			return NETWORK_ERROR; // headerĳ���ÿ� �����Ͽ��ų� �޽��� ũ�Ⱑ 0 �̴�.
		}
		
		int resultSize = headerSize + header.BodySize;
		if (_bufferSize < resultSize)
		{
			return NETWORK_ERROR; // ��ü �޽����� ���� �������� ����
		}

		LockOff();

		outBuffer = new char[resultSize];
		bufferSize = resultSize;
		std::memcpy(outBuffer, _buffer, bufferSize);

		int rebalanceSize = _bufferSize - bufferSize;
		_bufferSize -= bufferSize;

		std::memmove(_buffer, _buffer + resultSize, rebalanceSize);

		LockOff();

		return NETWORK_OK;
	}
}