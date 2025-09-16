#pragma once
#include "../Network/NetworkBaseServer.h"
#include "FieldServerDefine.h"
#include "Player.h"
#include <winhttp.h>
#include "../ThirdParty/protocol/SERVER_PROTOCOL_generated.h"
#include "oneapi-tbb-2022.2.0/include/oneapi/tbb/concurrent_queue.h"
#include "oneapi-tbb-2022.2.0/include/oneapi/tbb/concurrent_map.h"


namespace Field
{
	class FieldServer : public Network::NetworkBaseServer
	{
	public:
		FieldServer();
		~FieldServer() override;

	private:
		tbb::concurrent_map<DWORD, Field::Player*> _playerMap;
		tbb::concurrent_queue<DWORD> _newUserQueue;

	public:
		int PowerOnSequence() override; // ���� ���� �� �ʱ�ȭ �۾��� �����մϴ�.
		int	WorkProcess() override;
		int	AcceptProcess() override;
		int	UpdateProcess() override;

	protected:
		int DisconnectClient(DWORD completionKey) override;
		void PlayerOnlineCheck(unsigned long long currentTime) override;
		void ProcessHeartBeat() override;

	private :
		void UpdateUserList();

	private:
		MessageDispatcher _messageDispatchers[protocol::MESSAGETYPE::MESSAGETYPE_MAX];
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

	private:
		void INNER_CLOSE_CLIENT(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage);
		void REQUEST_REGISTER(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage);
		void RESPONSE_HEARTBEAT(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage);
		void REQUEST_PLAYERMOVE(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage);
	};
}