#pragma once
#include "pch.h"
#include "WinSock2.h"

#define NETWORK_OK			  1
#define NETWORK_ERROR		 -1

#define MAX_IP_LENGTH 20

#define CLIENT_MAX 1000 // 최대 클라이언트 수

// IOCP Worker thread number per CPU 
#define RATIO_CPU_THREAD 2

#define THREAD_MAX 15

#define WORKER_THREAD_MAX 10

#define SAFE_CLOSE_HANDLE(P) { if(P) { CloseHandle(P); P = INVALID_HANDLE_VALUE; } }

#define NET_DATA_BUFSIZE 2048

namespace Network
{
    enum OperationType
    {
        OP_DEFAULT = 0,

        OP_RECV = 1,
        OP_SEND = 2,

		OP_END = 3,
    };

    enum SenderType
    {
        DEFAULT = 0,
        CONTROL_SERVER = 1,
        AUTH_SERVER = 2,
        LOBBY_SERVER = 3,
        CLIENT = 4
    };

#pragma pack(push, 1)
    struct MessageHeader
    {
        uint32_t BodySize;
        uint32_t ContentsType;

        MessageHeader()
        {
            BodySize = 0;
            ContentsType = 0;
        }

        MessageHeader(uint32_t bodySize, uint32_t contentsType) : BodySize(bodySize), ContentsType(contentsType)
        {
        }

        MessageHeader(const MessageHeader& other) : BodySize(other.BodySize), ContentsType(other.ContentsType)
        {
        }
    };
#pragma pack(pop)

    struct MessageData
    {
    public:
        DWORD CompletionKey;
        MessageHeader Header;
        char* Body;
        int BodySize;

        MessageData()
        {
            CompletionKey = -1;
            Body = nullptr;
            BodySize = -1;
        }

		// MessageBuilder에서 동적할당된 body를 이용하여 MessageData 객체를 생성
        MessageData(DWORD completionKey, char* messageBuffer)
        {
            if (messageBuffer != nullptr)
            {
                std::memcpy(&Header, messageBuffer, sizeof(MessageHeader));
                if (Header.BodySize > 0)
                {
                    CompletionKey = completionKey;
                    BodySize = Header.BodySize;
                    Body = messageBuffer;
                    return;
                }
            }

            CompletionKey = -1;
            Body = nullptr;
            BodySize = -1;
        }

		//Flatbuffers 메시지 헤더와 바디를 이용하여 MessageData 객체를 생성
        MessageData(DWORD completionKey, MessageHeader header, char* body)
        {
            CompletionKey = completionKey;
            BodySize = header.BodySize;
			Header = header;

            std::memcpy(Body, body, BodySize);
        }

        int OverlappedSize()
        {
			return sizeof(MessageHeader) + BodySize;
        }
    };

    struct CustomOverlapped : OVERLAPPED
    {
    public:
        WSABUF Wsabuf;
        CHAR Buffer[NET_DATA_BUFSIZE];
        OperationType OperationType;
        DWORD CompletionKey;

        CustomOverlapped()
        {
            CompletionKey = 0;
            Wsabuf.len = 0;
            Wsabuf.buf = Buffer;
            OperationType = OP_DEFAULT;
            this->hEvent = NULL;
        }

        void CopyFromMessageData(Network::MessageData& messageData)
        {
            Wsabuf.len = messageData.OverlappedSize();

            std::memcpy(Wsabuf.buf, &messageData.Header, sizeof(MessageHeader));
            std::memcpy(Wsabuf.buf + sizeof(MessageHeader), messageData.Body, messageData.BodySize);
        }

        void Clear()
        {
			std::memset(Wsabuf.buf, 0, sizeof(Wsabuf.len));
            Wsabuf.len = 0;
            OperationType = OP_DEFAULT;
            CompletionKey = 0;
            this->hEvent = NULL;
        }
    };

}
