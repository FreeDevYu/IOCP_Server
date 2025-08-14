#pragma once
#include "../Network/NetworkBaseServer.h"
#include "FieldServerDefine.h"
#include "Player.h"
#include <winhttp.h>

#include "oneTBB/include/oneapi/tbb/concurrent_queue.h"
#include "oneTBB/include/oneapi/tbb/concurrent_map.h"

namespace Field
{
	class FieldServer : public Network::NetworkBaseServer
	{
	public:
		FieldServer();
		~FieldServer() override;

	private:
		tbb::concurrent_map<DWORD, Field::Player*> _playerMap;

	public:
		int PowerOnSequence() override; // ���� ���� �� �ʱ�ȭ �۾��� �����մϴ�.
		int	WorkProcess() override;
		int	AcceptProcess() override;
		int	UpdateProcess() override;

	protected:
		int DisconnectClient(DWORD completionKey) override;
		void PlayerOnlineCheck(unsigned long long currentTime) override;
		void ProcessHeartBeat() override;

	private:
		tbb::concurrent_queue<std::shared_ptr<Network::MessageData>> _messageQueue;
		void RecvMessageProcess();
		void ReadMessage(std::shared_ptr<Network::MessageData> messageData);
		int RegistMessageDispatcher();

	public:
		std::function<void(const std::string&, const std::string&)> _debugLogCallback;
		void SetDebugLogCallback(std::function<void(const std::string&, const std::string&)> callback);

	protected:
		void DebugLog(Debug::DebugType debugtype, const std::string& message) override;

	private:
		std::unordered_map<std::string, std::function<void(const std::vector<std::string>)>> _commandMap;

		void SettingExternalCommands();
		void ReceiveExternalCommand(std::string& input);
		void SendTelegramMessage(const std::string& message);
	};
}