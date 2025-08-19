#include "FieldServer.h"

namespace Field
{
	void FieldServer::INNER_CLOSE_CLIENT(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage)
	{
		Field::FieldServer* fieldServer = static_cast<Field::FieldServer*>(&server);
		if (fieldServer == nullptr)
			return;

		auto innerCloseClient = flatbuffers::GetRoot<protocol::INNER_CLOSE_CLIENT>(receiveMessage->Body);

		if (innerCloseClient == nullptr)
		{
			DebugLog(Debug::DEBUG_ERROR, "INNER_CLOSE_CLIENT: Invalid request data.");
			return;
		}
		DWORD completionKey = receiveMessage->CompletionKey;
		DebugLog(Debug::DEBUG_LOG, std::format("Receive [INNER_CLOSE_CLIENT] from player: {}", completionKey));

		int result = DisconnectClient(completionKey);

		if (result != NETWORK_OK)
		{
			DebugLog(Debug::DEBUG_ERROR, std::format("DisconnectClient failed for completionKey: {}", completionKey));
			return;
		}

		DebugLog(Debug::DEBUG_LOG, std::format("Player disconnected: {} Complete", completionKey));
	}

	void FieldServer::REQUEST_REGISTER(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage)
	{
		Field::FieldServer* fieldServer = static_cast<Field::FieldServer*>(&server);
		if (fieldServer == nullptr)
			return;

		bool success = true;

		auto requestConnect = flatbuffers::GetRoot<protocol::REQUEST_REGISTER>(receiveMessage->Body);
		if( requestConnect == nullptr)
		{
			DebugLog(Debug::DEBUG_ERROR, "REQUEST_REGISTER: Invalid request data.");
			success = false;
		}

		DWORD completionKey = receiveMessage->CompletionKey;
		Network::NetworkUser* networkUser = fieldServer->GetNetworkUser(completionKey);

		auto dulplicatePlayerChecker = fieldServer->_playerMap.find(completionKey);
		if (dulplicatePlayerChecker == fieldServer->_playerMap.end())
		{
			// 요청플레이어 탐색 실패
			success = false;
		}

		if (success)
		{
			// 플레이어 등록
			std::string playerID = requestConnect->player_id()->str();
			dulplicatePlayerChecker->second->Register(playerID);

			DebugLog(Debug::DEBUG_LOG, std::format("Receive [REQUEST_REGISTER] {}", playerID));
			SendTelegramMessage(std::format("Receive REQUEST_REGISTER from player {}", playerID));
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

	void FieldServer::RESPONSE_HEARTBEAT(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage)
	{
		Field::FieldServer* fieldServer = static_cast<Field::FieldServer*>(&server);
		if (fieldServer == nullptr)
			return;

		DWORD completionKey = receiveMessage->CompletionKey;
		Network::NetworkUser* networkUser = fieldServer->GetNetworkUser(completionKey);

		auto finder = fieldServer->_playerMap.find(completionKey);
		if (finder == fieldServer->_playerMap.end())
			return; // 해당 플레이어가 존재하지 않음

		_playerMap[completionKey]->SaveResponseHeartBeat();

		DebugLog(Debug::DEBUG_LOG, std::format("Receive [RESPONSE_HEARTBEAT] from player: {}", _playerMap[completionKey]->GetCompletionKey()));
	}
}