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
				// �̹� �����ϴ� �÷��̾��� ��� ó�� ���� �߰� �ʿ�
				success = false;
			}
		}

		if (success)
		{
			// ���ο� �÷��̾� �߰�
			Manager::Player* newPlayer = new Manager::Player();
			newPlayer->Initialize("Name", completionKey);
			serverManager->_playerMap.insert({ completionKey, newPlayer });
		}


		flatbuffers::FlatBufferBuilder builder;
		builder.Finish(protocol::CreateRESPONSE_CONNECT(builder, success));

		Network::MessageHeader header(1, builder.GetSize(), protocol::MESSAGETYPE::MESSAGETYPE_RESPONSE_CONNECT);// sendertype �ʿ��Ѱ�?
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

	}
	void ServerManager::RESPONSE_DISCONNECT(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage)
	{

	}

	void ServerManager::NOTICE_KICK(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage)
	{

	}

	void ServerManager::REQUEST_HEARTBEAT(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage)
	{

	}

	void ServerManager::RESPONSE_HEARTBEAT(Network::NetworkBaseServer& server, std::shared_ptr<Network::MessageData> receiveMessage)
	{

	}
}