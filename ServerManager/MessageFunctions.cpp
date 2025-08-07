#include "ServerManager.h"

namespace Manager
{
	void ServerManager::REQUEST_REGISTER(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage)
	{
		Manager::ServerManager* serverManager = static_cast<Manager::ServerManager*>(&server);
		bool success = true;
		if (serverManager == nullptr)
			return;

		auto requestConnect = flatbuffers::GetRoot<protocol::REQUEST_REGISTER>(receiveMessage->Body);
		if( requestConnect == nullptr)
		{
			DebugLog(Debug::DEBUG_ERROR, "REQUEST_REGISTER: Invalid request data.");
			success = false;
		}

		DWORD completionKey = receiveMessage->CompletionKey;
		Network::NetworkUser* networkUser = serverManager->GetNetworkUser(completionKey);

		auto ipChecker = serverManager->_serverIpSet.find(networkUser->GetIpAddress());
		if (ipChecker == serverManager->_serverIpSet.end())
		{
			success = false;
		}
		else
		{
			auto dulplicatePlayerChecker = serverManager->_playerMap.find(completionKey);
			if (dulplicatePlayerChecker == serverManager->_playerMap.end())
			{
				// 요청플레이어 탐색 실패
				success = false;
			}

			if (success)
			{
				// 플레이어 등록
				std::string serverName = requestConnect->server_name()->str();
				dulplicatePlayerChecker->second->Register(serverName);

				DebugLog(Debug::DEBUG_LOG, std::format("Receive [REQUEST_REGISTER] {}", serverName));
				SendTelegramMessage(std::format("Receive REQUEST_REGISTER from player {}", serverName));
			}
		}
		
		flatbuffers::FlatBufferBuilder builder;
		builder.Finish(protocol::CreateRESPONSE_REGISTER(builder, success));

		Network::MessageHeader header(builder.GetSize(), protocol::MESSAGETYPE::MESSAGETYPE_RESPONSE_REGISTER);
		std::shared_ptr<Network::MessageData> messageData = std::make_shared<Network::MessageData>(
			completionKey,
			header,
			(char*)builder.GetBufferPointer()
		);

		SendMessageToClient(completionKey, messageData);
		DebugLog(Debug::DEBUG_LOG, std::format("Send [RESPONSE REGISTER] for player: {}", networkUser->GetIpAddress()));
	}

	void ServerManager::RESPONSE_HEARTBEAT(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage)
	{
		Manager::ServerManager* serverManager = static_cast<Manager::ServerManager*>(&server);
		if (serverManager == nullptr)
			return;

		DWORD completionKey = receiveMessage->CompletionKey;
		Network::NetworkUser* networkUser = serverManager->GetNetworkUser(completionKey);

		auto finder = serverManager->_playerMap.find(completionKey);
		if (finder == serverManager->_playerMap.end())
			return; // 해당 플레이어가 존재하지 않음

		_playerMap[completionKey]->SaveRequestHearbeatTime();

		DebugLog(Debug::DEBUG_LOG, std::format("Receive [RESPONSE_HEARTBEAT] from player: {}", _playerMap[completionKey]->GetServerName()));
		SendTelegramMessage(std::format("Receive RESPONSE_HEARTBEAT from player {}", _playerMap[completionKey]->GetServerName()));
	}
}