#include "ServerManager.h"

namespace Manager
{
	void ServerManager::REQUEST_CONNECT(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage)
	{
		Manager::ServerManager* serverManager = static_cast<Manager::ServerManager*>(&server);
		bool success = true;
		if (serverManager == nullptr)
			return;

		DWORD completionKey = receiveMessage->CompletionKey;
		Network::NetworkUser* networkUser = serverManager->GetNetworkUser(completionKey);

		auto ipChecker = serverManager->_serverIpSet.find(networkUser->GetIpAddress());
		if(ipChecker == serverManager->_serverIpSet.end())
		{
			success = false;
		}
		else
		{
			auto dulplicatePlayerChecker = serverManager->_playerMap.find(completionKey);
			if (dulplicatePlayerChecker != serverManager->_playerMap.end())
			{
				// 이미 존재하는 플레이어의 경우 처리 로직 추가 필요
				success = false;
			}
		}

		if (success)
		{
			// 새로운 플레이어 추가
			Manager::Player* newPlayer = new Manager::Player();
			newPlayer->Initialize("Name", completionKey);
			serverManager->_playerMap.insert({ completionKey, newPlayer });
		}


		flatbuffers::FlatBufferBuilder builder;
		builder.Finish(protocol::CreateRESPONSE_CONNECT(builder, success));

		Network::MessageHeader header(1, builder.GetSize(), protocol::MESSAGETYPE::MESSAGETYPE_RESPONSE_CONNECT);// sendertype 필요한가?
		std::shared_ptr<Network::MessageData> messageData = std::make_shared<Network::MessageData>(
			completionKey,
			header,
			(char*)builder.GetBufferPointer()
		);

		SendMessageToClient(completionKey, messageData);
	}

	void ServerManager::RESPONSE_CONNECT(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage)
	{

	}

	void ServerManager::REQUEST_DISCONNECT(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage)
	{
		//completionkey를 통해 메세지 생성후 메세지큐에 직접넣기.
	}
	void ServerManager::RESPONSE_DISCONNECT(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage)
	{

	}

	void ServerManager::NOTICE_KICK(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage)
	{

	}

	void ServerManager::REQUEST_HEARTBEAT(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage)
	{
		Manager::ServerManager* serverManager = static_cast<Manager::ServerManager*>(&server);
		if (serverManager == nullptr)
			return;

		DWORD completionKey = receiveMessage->CompletionKey;
		Network::NetworkUser* networkUser = serverManager->GetNetworkUser(completionKey);

		auto finder = serverManager->_playerMap.find(completionKey);
		if(finder == serverManager->_playerMap.end())
			return; // 해당 플레이어가 존재하지 않음

		_playerMap[completionKey]->RequestHeartBeat();

	}

	void ServerManager::RESPONSE_HEARTBEAT(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage)
	{

	}
}