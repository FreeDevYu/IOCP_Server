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
        uint32_t SenderType;
        uint32_t BodySize;
        uint32_t ContentsType;

        MessageHeader()
        {
            SenderType = 0;
            BodySize = 0;
            ContentsType = 0;
        }

        MessageHeader(uint32_t senderType, uint32_t bodySize, uint32_t contentsType) : SenderType(senderType), BodySize(bodySize), ContentsType(contentsType)
        {
        }

        MessageHeader(const MessageHeader& other) : SenderType(other.SenderType), BodySize(other.BodySize), ContentsType(other.ContentsType)
        {
        }
    };
#pragma pack(pop)


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

        void Clear()
        {
            Wsabuf.len = 0;
            Wsabuf.buf = nullptr;
            OperationType = OP_DEFAULT;
			CompletionKey = 0;
            this->hEvent = NULL;
        }
    };

    struct MessageData
    {
    public:
        DWORD CompletionKey;
        MessageHeader Header;
        std::string Body;
        int BodySize;

        MessageData()
        {
            CompletionKey = -1;
            Body = "";
            BodySize = -1;
        }

        MessageData(DWORD completionKey, char* messageBuffer)
        {
            if (messageBuffer != nullptr)
            {
                std::memcpy(&Header, messageBuffer, sizeof(MessageHeader));
                if (Header.BodySize > 0)
                {
                    CompletionKey = completionKey;
                    BodySize = Header.BodySize;
                    Body = std::string(messageBuffer + sizeof(MessageHeader), BodySize); // string복사를  MessageData객체와 생명주기 일치시킴 -> 안정성확보
                    return;
                }
            }

            CompletionKey = -1;
            Body = "";
            BodySize = -1;
        }
    };
}
