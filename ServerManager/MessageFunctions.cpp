#include "ServerManager.h"

namespace Manager
{
	void ServerManager::REQUEST_CONNECT(Network::NetworkBaseServer& server, DWORD completionKey, std::string)
	{
		Manager::ServerManager* serverManager = static_cast<Manager::ServerManager*>(&server);
		if (serverManager == nullptr)
			return;

		Network::NetworkUser* networkUser = serverManager->GetNetworkUser(completionKey);

		auto ipChecker = serverManager->_serverIpSet.find(networkUser->GetIpAddress());
		if(ipChecker == serverManager->_serverIpSet.end())
		{
			return;
		}

		auto dulplicatePlayerChecker = serverManager->_playerMap.find(completionKey);
		if(dulplicatePlayerChecker != serverManager->_playerMap.end())
		{
			// �̹� �����ϴ� �÷��̾��� ��� ó�� ���� �߰� �ʿ�
			return;
		}

		// ���ο� �÷��̾� �߰�
		Manager::Player* newPlayer = new Manager::Player();
		newPlayer->Initialize("Name", completionKey);
		serverManager->_playerMap.insert({ completionKey, newPlayer });
	}
	void ServerManager::RESPONSE_CONNECT(Network::NetworkBaseServer& server, DWORD completionKey, std::string)
	{

	}

	void ServerManager::REQUEST_DISCONNECT(Network::NetworkBaseServer& server, DWORD completionKey, std::string)
	{

	}
	void ServerManager::RESPONSE_DISCONNECT(Network::NetworkBaseServer& server, DWORD completionKey, std::string)
	{

	}

	void ServerManager::NOTICE_KICK(Network::NetworkBaseServer& server, DWORD completionKey, std::string)
	{

	}

	void ServerManager::REQUEST_HEARTBEAT(Network::NetworkBaseServer& server, DWORD completionKey, std::string)
	{

	}

	void ServerManager::RESPONSE_HEARTBEAT(Network::NetworkBaseServer& server, DWORD completionKey, std::string)
	{

	}
}