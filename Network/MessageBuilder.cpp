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

	int MessageBuilder::MessageCheckAndReturn(Network::MessageHeader& header, char*& bodyBuffer)
	{
		int headerSize = sizeof(MessageHeader);
	
		if(_bufferSize < headerSize)
		{
			return NETWORK_ERROR; // ���ۿ� �޽����� ����.
		}

		LockOn();

		std::memcpy(&header, _buffer, sizeof(MessageHeader));

		if(header.BodySize <= 0 || header.BodySize > NET_DATA_BUFSIZE)
		{
			LockOff();
			return NETWORK_ERROR; // headerĳ���ÿ� �����Ͽ��ų� �޽��� ũ�Ⱑ 0 �̴�.
		}
		
		int resultSize = headerSize + header.BodySize;
		if (_bufferSize < resultSize)
		{
			LockOff();
			return NETWORK_ERROR; // ��ü �޽����� ���� �������� ����
		}

		bodyBuffer = new char[header.BodySize];

		std::memcpy(bodyBuffer, _buffer + sizeof(MessageHeader), header.BodySize);

		int sliceSize = sizeof(MessageHeader) + header.BodySize;
		_bufferSize -= sliceSize;

		std::memmove(_buffer, _buffer + resultSize, _bufferSize);

		LockOff();

		return NETWORK_OK;
	}
}