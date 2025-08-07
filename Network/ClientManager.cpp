#pragma once
#include "ClientManager.h"

namespace Network
{
	ClientManager::ClientManager()
	{
	}

	ClientManager::~ClientManager()
	{
	}

	void ClientManager::InitializeBase(int maxClient)
	{
		_maxClient = maxClient;
		_clientMap.clear();
		_completionKeyQueue.clear();
		
		for (int i = 1; i <= _maxClient; ++i) //0�� �ڱ��ڽ�?
		{
			_completionKeyQueue.push(i);
		}
	}

	DWORD ClientManager::PopCompletionKey()
	{
		if (_currentClientCount.load() >= _maxClient)
		{
			return NETWORK_ERROR; // �ִ� Ŭ���̾�Ʈ �� �ʰ�
		}

		if(_completionKeyQueue.empty())
		{
			return NETWORK_ERROR; // ť�� ������� ���
		}

		DWORD completionKey;
		if (_completionKeyQueue.try_pop(completionKey))
		{
			return completionKey;
		}
		else
		{
			return NETWORK_ERROR; // ť�� ������� ���
		}
	}

	Network::NetworkUser* ClientManager::GetNetworkUser(DWORD completionKey)
	{
		if (completionKey < _maxClient)
			return _clientMap[completionKey];
		else
			return nullptr;
	}

	int ClientManager::AddClient(DWORD completionKey, Network::NetworkUser* user)
	{
		if (completionKey == NETWORK_ERROR || user == nullptr)
		{
			return NETWORK_ERROR; // �߸��� �Ķ����
		}

		if (_currentClientCount.load() >= _maxClient)
		{
			return NETWORK_ERROR; // �ִ� Ŭ���̾�Ʈ �� �ʰ�
		}

		if (_clientMap.find(completionKey) != _clientMap.end())
		{
			return NETWORK_ERROR; // �̹� �����ϴ� Ŭ���̾�Ʈ
		}

		//user->Initialize(completionKey, INVALID_SOCKET); // ������ ���� �������� ����
		_clientMap[completionKey] = user;
		_currentClientCount.fetch_add(1);
		
		return NETWORK_OK;
	}

	int ClientManager::RemoveClient(DWORD completionKey)
	{
		if (completionKey == NETWORK_ERROR)
		{
			return NETWORK_ERROR; // �߸��� �Ķ����
		}

		auto iterator = _clientMap.find(completionKey);
		if (iterator == _clientMap.end())
		{
			return NETWORK_ERROR; // �������� �ʴ� Ŭ���̾�Ʈ
		}

		//delete iterator->second; // NetworkUser ��ü ����
		LockOn();
		_clientMap.unsafe_erase(iterator); // Ŭ���̾�Ʈ ��Ͽ��� ����
		LockOff();

		_currentClientCount.fetch_sub(1);
		_completionKeyQueue.push(completionKey); // ť�� completionKey ��ȯ
		
		return NETWORK_OK;
	}

	int ClientManager::AddMessageToClient(DWORD completionKey, void* message, DWORD size)
	{
		if (completionKey < _maxClient)
		{
			_clientMap[completionKey]->AddMessage(message, size);

			return NETWORK_OK; // ���������� �޽����� �߰�
		}
		else
		{
			return NETWORK_ERROR; // �߸��� completionKey
		}
	}

	std::shared_ptr<Network::MessageData> ClientManager::GetReceiveMessageFromClient(DWORD completionKey)
	{
		if (completionKey < _maxClient)
		{
			return _clientMap[completionKey]->GetReceiveMessage();
		}
		
		return nullptr;
	}

}