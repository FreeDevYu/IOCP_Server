#pragma once
#include <atomic>
#include "winsock2.h"
#include "pch.h"
#include "Lock.h"
#include "MessageBuilder.h"

namespace Network
{
	class NetworkUser : public DefaultLock
	{
	public:
		NetworkUser();
		~NetworkUser();

	protected:
		MessageBuilder* _messageBuilder;
		DWORD _completionKey; // unsinged int : 0 ~ 4,294,967,295
		SOCKET _socket;
		std::string _ip;
		int _port;

		std::atomic<int> _ioPendingCount; // IO 작업 대기 중인 횟수
		std::atomic<DWORD> _ioLastRequeueTime; // 마지막 IO 요청 시간

	public:
		void Initialize(DWORD completionKey, SOCKET socket, std::string ip, int port);
		void Deinitialize();

		SOCKET GetSocket();
		DWORD GetCompletionKey() const;

		int AddMessage(void* message, DWORD size);
		std::shared_ptr<Network::MessageData> GetReceiveMessage();

		inline bool IsOnline() { return _socket == INVALID_SOCKET ? false : true; }

		int GetPendingIOCount() const;
		void IncreasePendingIOCount();
		void DecreasePendingIOCount();

		void SetLastRequeueTime(DWORD time);
		DWORD GetLastRequeueTime() const;

		std::string GetIpAddress() const;
	};
}
