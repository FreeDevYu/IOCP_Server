#pragma once
#include <atomic>
#include "tbb/concurrent_queue.h"
#include "tbb/concurrent_unordered_map.h"
#include "NetworkUser.h"
#include "NetworkDefine.h"
#include "Lock.h"


namespace Network
{
	class ClientManager : public DefaultLock
	{
		using ClientMap = tbb::concurrent_unordered_map<DWORD, Network::NetworkUser*>;
		using CompletionKeyQueue = tbb::concurrent_queue<DWORD>;

	public:
		ClientManager();
		virtual ~ClientManager();

	protected:
		ClientMap _clientMap; // 클라이언트 목록
		int _maxClient; // 최대 클라이언트 수
		std::atomic<int> _currentClientCount; // 현재 클라이언트 수
		CompletionKeyQueue _completionKeyQueue;

	public:
		void InitializeBase(int maxClient);
		DWORD PopCompletionKey();
		Network::NetworkUser* GetNetworkUser(DWORD completionKey);

		int AddClient(DWORD completionKey, Network::NetworkUser* user);
		int RemoveClient(DWORD completionKey);

		int AddMessageToClient(DWORD completionKey, void* message, DWORD size);
		std::shared_ptr<Network::MessageData> GetReceiveMessageFromClient(DWORD completionKey);

	};
} 