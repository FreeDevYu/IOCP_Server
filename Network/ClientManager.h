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
		ClientMap _clientMap; // Ŭ���̾�Ʈ ���
		int _maxClient; // �ִ� Ŭ���̾�Ʈ ��
		std::atomic<int> _currentClientCount; // ���� Ŭ���̾�Ʈ ��
		CompletionKeyQueue _completionKeyQueue;


	public:
		void InitializeBase(int maxClient);
		DWORD PopCompletionKey();
		Network::NetworkUser* GetNetworkUser(DWORD completionKey);

		int AddClient(DWORD completionKey, Network::NetworkUser* user);
		int RemoveClient(DWORD completionKey);

		int AddMessageToClient(DWORD completionKey, void* message, DWORD size);
		void* GetReceiveMessageFromClient(DWORD completionKey);

	};
} 