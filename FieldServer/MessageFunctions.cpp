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

		DWORD targetCompletionKey = receiveMessage->CompletionKey;
		auto it = fieldServer->_playerMap.find(targetCompletionKey);
		if(it == fieldServer->_playerMap.end())
		{
			// 요청플레이어 탐색 실패
			DebugLog(Debug::DEBUG_ERROR, std::format("INNER_CLOSE_CLIENT: Player not found for completionKey {}", targetCompletionKey));
			return;
		}

		std::string targetPlayerID = _playerMap[targetCompletionKey]->GetPlayerID();
		DebugLog(Debug::DEBUG_LOG, std::format("Receive [INNER_CLOSE_CLIENT] from player: {}", targetCompletionKey));

		for (auto& [key, player] : _playerMap)
		{
			if (player == nullptr) continue;

			if (player->GetCompletionKey() == targetCompletionKey)
			{
				SendTelegramMessage(std::format("Player disconnected: {} ", player->GetPlayerID()));
			}
			else
			{
				flatbuffers::FlatBufferBuilder builder;
				auto playerIdOFffset = builder.CreateString(targetPlayerID);
				builder.Finish(protocol::CreateNOTICE_EXIT_STAGE(builder, playerIdOFffset));

				Network::MessageHeader header(builder.GetSize(), protocol::MESSAGETYPE::MESSAGETYPE_NOTICE_EXIT_STAGE);
				std::shared_ptr<Network::MessageData> messageData = std::make_shared<Network::MessageData>(
					player->GetCompletionKey(),
					header,
					(char*)builder.GetBufferPointer()
				);

				SendMessageToClient(player->GetCompletionKey(), messageData);
				DebugLog(Debug::DEBUG_LOG, std::format("Send [NOTICE_EXIT_STAGE] To Player {}.", player->GetPlayerID()));
			}
		}

		int result = DisconnectClient(targetCompletionKey);

		if (result != NETWORK_OK)
		{
			DebugLog(Debug::DEBUG_ERROR, std::format("DisconnectClient failed for completionKey: {}", targetCompletionKey));
			return;
		}

		DebugLog(Debug::DEBUG_LOG, std::format("Player disconnected: {} Complete", targetCompletionKey));
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

		std::string targetPlayerID = requestConnect->player_id()->str();
		DWORD completionKey = receiveMessage->CompletionKey;

		auto dulplicatePlayerChecker = fieldServer->_playerMap.find(completionKey);
		if (dulplicatePlayerChecker == fieldServer->_playerMap.end())
		{
			// 요청플레이어 탐색 실패
			success = false;
		}

		// sometthing regist check...
		
		if (success)
		{
			// 플레이어 등록
			auto newEntrancePlayer = dulplicatePlayerChecker->second;
			
			newEntrancePlayer->Register(targetPlayerID);
			_newUserQueue.push(completionKey);
			DebugLog(Debug::DEBUG_LOG, std::format("Receive [REQUEST_REGISTER] {}", targetPlayerID));
			SendTelegramMessage(std::format("Receive REQUEST_REGISTER from player {}", targetPlayerID));
		}
		else
		{
			DebugLog(Debug::DEBUG_ERROR, std::format("REQUEST_REGISTER failed for player: {}", targetPlayerID));
		}

		flatbuffers::FlatBufferBuilder builder;
		auto playerIDOffset = builder.CreateString(targetPlayerID);
		builder.Finish(protocol::CreateRESPONSE_REGISTER(builder, playerIDOffset, success));
		Network::MessageHeader header(builder.GetSize(), protocol::MESSAGETYPE::MESSAGETYPE_RESPONSE_REGISTER);
		std::shared_ptr<Network::MessageData> messageData = std::make_shared<Network::MessageData>(
			completionKey,
			header,
			(char*)builder.GetBufferPointer()
		);

		if (success)
		{
			

			// 신규 플레이어에게 기존 접속자들정보를 전달한다.
			// 기존 접속자들에게 신규 플레이어 입장정보를 전달한다.

		//	for (auto& [key, player] : _playerMap)
		//	{
		//		if (player == nullptr) continue;
		//
		//		if (player == nullptr)
		//			continue;
		//
		//		// 기존 접속자에게 신규 접속자 정보를 전달한다.
		//		if (player->GetCompletionKey() != newEntrancePlayer->GetCompletionKey())
		//		{
		//			flatbuffers::FlatBufferBuilder builder;
		//			auto playerIdOFffset = builder.CreateString(newEntrancePlayer->GetPlayerID());
		//			builder.Finish(protocol::CreateNOTICE_ENTRANCE_STAGE(builder, playerIdOFffset,
		//				newEntrancePlayer->_xPosition, newEntrancePlayer->_yPosition, newEntrancePlayer->_zPosition));
		//
		//			Network::MessageHeader header(builder.GetSize(), protocol::MESSAGETYPE::MESSAGETYPE_NOTICE_ENTRANCE_STAGE);
		//			std::shared_ptr<Network::MessageData> messageData = std::make_shared<Network::MessageData>(
		//				player->GetCompletionKey(),
		//				header,
		//				(char*)builder.GetBufferPointer()
		//			);
		//
		//			SendMessageToClient(player->GetCompletionKey(), messageData);
		//			DebugLog(Debug::DEBUG_LOG, std::format("Send [NOTICE_ENTRANCE_STAGE] To Player {}.", player->GetPlayerID()));
		//		}
		//
		//		// 신규 접속자에게 기존 접속자 정보를 전달한다.
		//		flatbuffers::FlatBufferBuilder builder;
		//		auto playerIdOFffset = builder.CreateString(player->GetPlayerID());
		//		builder.Finish(protocol::CreateNOTICE_ENTRANCE_STAGE(builder, playerIdOFffset,
		//			player->_xPosition, player->_yPosition, player->_zPosition));
		//
		//		Network::MessageHeader header(builder.GetSize(), protocol::MESSAGETYPE::MESSAGETYPE_NOTICE_ENTRANCE_STAGE);
		//		std::shared_ptr<Network::MessageData> messageData = std::make_shared<Network::MessageData>(
		//			newEntrancePlayer->GetCompletionKey(),
		//			header,
		//			(char*)builder.GetBufferPointer()
		//		);
		//
		//		SendMessageToClient(newEntrancePlayer->GetCompletionKey(), messageData);
		//	}
		//
		//	DebugLog(Debug::DEBUG_LOG, std::format("Send [NOTICE_ENTRANCE_STAGE] To Player {}.", newEntrancePlayer->GetPlayerID()));
		}
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

	void FieldServer::REQUEST_PLAYERMOVE(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage)
	{
		Field::FieldServer* fieldServer = static_cast<Field::FieldServer*>(&server);
		if (fieldServer == nullptr)
			return;
		auto requestPlayerMove = flatbuffers::GetRoot<protocol::REQUEST_PLAYERMOVE>(receiveMessage->Body);
		if (requestPlayerMove == nullptr)
		{
			DebugLog(Debug::DEBUG_ERROR, "REQUEST_PLAYERMOVE: Invalid request data.");
			return;
		}

		DWORD completionKey = receiveMessage->CompletionKey;

		Network::NetworkUser* networkUser = fieldServer->GetNetworkUser(completionKey);
		auto finder = fieldServer->_playerMap.find(completionKey);
		if (finder == fieldServer->_playerMap.end())
			return; // 해당 플레이어가 존재하지 않음

		Field::Player* player = _playerMap[completionKey];
		if (player == nullptr)
			return;

		float angleDeg = 0.0f;
		player->MovePosition(requestPlayerMove->direction(), requestPlayerMove->duration(), angleDeg);
		DebugLog(Debug::DEBUG_LOG, std::format("Receive [REQUEST_PLAYERMOVE] from player: {}", _playerMap[completionKey]->GetCompletionKey()));

		flatbuffers::FlatBufferBuilder builder;
		auto playerIDOffset = builder.CreateString(requestPlayerMove->player_id()->str());
		builder.Finish(protocol::CreateNOTICE_PLAYERPOSITION(builder, playerIDOffset, 
			requestPlayerMove->direction(), player->GetMoveSpeed(), requestPlayerMove->duration(),
			player->_xPosition, player->_yPosition, player->_zPosition));


		for (auto& [key, player] : _playerMap)
		{
			if (player == nullptr) continue;
			
			if (player == nullptr)
				continue;

			Network::MessageHeader header(builder.GetSize(), protocol::MESSAGETYPE::MESSAGETYPE_NOTICE_PLAYERPOSITION);
			std::shared_ptr<Network::MessageData> messageData = std::make_shared<Network::MessageData>(
				player->GetCompletionKey(),
				header,
				(char*)builder.GetBufferPointer()
			);

			SendMessageToClient(player->GetCompletionKey(), messageData);
		}

		DebugLog(Debug::DEBUG_LOG, std::format("Send [NOTICE PLAYERPOSITION] for player: {}", networkUser->GetIpAddress()));
	}
}