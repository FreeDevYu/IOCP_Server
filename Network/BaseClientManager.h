#pragma once
#include <atomic>
#include "tbb/concurrent_queue.h"
#include "tbb/concurrent_unordered_map.h"
#include "NetworkUser.h"
#include "NetworkDefine.h"
#include "Lock.h"


namespace Network
{
	class BaseClientManager : public DefaultLock
	{
		typedef tbb::concurrent_unordered_map<DWORD, NetworkUser*> ClientMap;
		typedef tbb::concurrent_queue<DWORD> CompletionKeyQueue;

	public:
		BaseClientManager();
		virtual ~BaseClientManager();

	protected:
		ClientMap _clientMap; // 클라이언트 목록
		int _maxClient; // 최대 클라이언트 수
		std::atomic<int> _currentClientCount; // 현재 클라이언트 수
		CompletionKeyQueue _completionKeyQueue;


	public:
		void InitializeBase(int maxClient);
		DWORD PopCompletionKey();
		SOCKET GetSocket(DWORD completionKey);

		int AddClient(DWORD completionKey, NetworkUser* user);
		int RemoveClient(DWORD completionKey);

		int AddMessageToClient(DWORD completionKey, void* message, DWORD size);
		void* GetReceiveMessageFromClient(DWORD completionKey);

	};
} 