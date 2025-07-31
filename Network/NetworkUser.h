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

		std::atomic<int> _ioPendingCount; // IO �۾� ��� ���� Ƚ��
		std::atomic<DWORD> _ioLastRequeueTime; // ������ IO ��û �ð�

	public:
		void Initialize(DWORD completionKey, SOCKET socket, std::string ip, int port);
		void Deinitialize();

		SOCKET GetSocket();
		DWORD GetCompletionKey() const;

		int AddMessage(void* message, DWORD size);
		void* GetReceiveMessage();

		inline bool IsOnline() { return _socket == INVALID_SOCKET ? false : true; }

		int GetPendingIOCount() const;
		void IncreasePendingIOCount();
		void DecreasePendingIOCount();

		void SetLastRequeueTime(DWORD time);
		DWORD GetLastRequeueTime() const;
	};
}
