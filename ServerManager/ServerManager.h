#pragma once

#include "../Network/NetworkBaseServer.h"
#include "oneTBB/include/oneapi/tbb/concurrent_queue.h"
#include "oneTBB/include/oneapi/tbb/concurrent_set.h"
#include "oneTBB/include/oneapi/tbb/concurrent_map.h"

#include "Player.h"

namespace Manager
{
	class ServerManager : public Network::NetworkBaseServer
	{
	public:
		ServerManager();
		~ServerManager() override;

	public:
		int	WorkProcess() override;
		int	AcceptProcess() override;
		int	UpdateProcess() override;
		int RegistMessageDispatcher() override;

	private:
		tbb::concurrent_queue<void*> _messageQueue;
		void RecvMessageProcess();
		void ReadMessage(void* message);

	private:
		tbb::concurrent_set<std::string> _serverIpSet;
		tbb::concurrent_map<DWORD, Manager::Player*> _playerMap;

	private:
		void REQUEST_CONNECT(Network::NetworkBaseServer& server, DWORD completionKey, std::string);
		void RESPONSE_CONNECT(Network::NetworkBaseServer& server, DWORD completionKey, std::string);

		void REQUEST_DISCONNECT(Network::NetworkBaseServer& server, DWORD completionKey, std::string);
		void RESPONSE_DISCONNECT(Network::NetworkBaseServer& server, DWORD completionKey, std::string);

		void NOTICE_KICK(Network::NetworkBaseServer& server, DWORD completionKey, std::string);

		void REQUEST_HEARTBEAT(Network::NetworkBaseServer& server, DWORD completionKey, std::string);
		void RESPONSE_HEARTBEAT(Network::NetworkBaseServer& server, DWORD completionKey, std::string);
	};

}
