#pragma once

#include "../Network/NetworkBaseServer.h"
#include "oneTBB/include/oneapi/tbb/concurrent_queue.h"
#include "oneTBB/include/oneapi/tbb/concurrent_set.h"
#include "oneTBB/include/oneapi/tbb/concurrent_map.h"
#include "ServerManagerDefine.h"
#include "Player.h"
#include <winhttp.h>




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
		tbb::concurrent_queue<std::shared_ptr<Network::MessageData>> _messageQueue;
		void RecvMessageProcess();
		void ReadMessage(std::shared_ptr<Network::MessageData> messageData);

	public:
		void AddServerIP(const std::string& ip);

	private:
		bool CheckServerIP(const std::string& ip) const;

	private:
		tbb::concurrent_set<std::string> _serverIpSet;
		tbb::concurrent_map<DWORD, Manager::Player*> _playerMap;

	private:
		void REQUEST_REGISTER(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage);
		void RESPONSE_HEARTBEAT(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage);

	private:
		void PlayerOnlineCheck(unsigned long long currentTime);
		void ProcessHeartBeat();

	protected:
		void DebugLog(Debug::DebugType debugtype, const std::string& message) override;

	public:
		std::function<void(const std::string&, const std::string&)> _debugLogCallback;
		void SetDebugLogCallback(std::function<void(const std::string&, const std::string&)> callback);

	public:
		void ReceiveExternalCommand(const std::string& command) override;

	private:
		std::unordered_map<std::string, std::function<void(const std::string&)>> _commandMap;

		void SettingExternalCommands();
		void SendTelegramMessage(const std::string& message);
	};
}
