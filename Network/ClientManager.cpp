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
		
		for (int i = 1; i <= _maxClient; ++i) //0은 자기자신?
		{
			_completionKeyQueue.push(i);
		}
	}

	DWORD ClientManager::PopCompletionKey()
	{
		if (_currentClientCount.load() >= _maxClient)
		{
			return NETWORK_ERROR; // 최대 클라이언트 수 초과
		}

		if(_completionKeyQueue.empty())
		{
			return NETWORK_ERROR; // 큐가 비어있을 경우
		}

		DWORD completionKey;
		if (_completionKeyQueue.try_pop(completionKey))
		{
			return completionKey;
		}
		else
		{
			return NETWORK_ERROR; // 큐가 비어있을 경우
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
			return NETWORK_ERROR; // 잘못된 파라미터
		}

		if (_currentClientCount.load() >= _maxClient)
		{
			return NETWORK_ERROR; // 최대 클라이언트 수 초과
		}

		if (_clientMap.find(completionKey) != _clientMap.end())
		{
			return NETWORK_ERROR; // 이미 존재하는 클라이언트
		}

		//user->Initialize(completionKey, INVALID_SOCKET); // 소켓은 아직 설정하지 않음
		_clientMap[completionKey] = user;
		_currentClientCount.fetch_add(1);
		
		return NETWORK_OK;
	}

	int ClientManager::RemoveClient(DWORD completionKey)
	{
		if (completionKey == NETWORK_ERROR)
		{
			return NETWORK_ERROR; // 잘못된 파라미터
		}

		auto iterator = _clientMap.find(completionKey);
		if (iterator == _clientMap.end())
		{
			return NETWORK_ERROR; // 존재하지 않는 클라이언트
		}

		//delete iterator->second; // NetworkUser 객체 삭제
		LockOn();
		_clientMap.unsafe_erase(iterator); // 클라이언트 목록에서 제거
		LockOff();

		_currentClientCount.fetch_sub(1);
		_completionKeyQueue.push(completionKey); // 큐에 completionKey 반환
		
		return NETWORK_OK;
	}

	int ClientManager::AddMessageToClient(DWORD completionKey, void* message, DWORD size)
	{
		if (completionKey < _maxClient)
		{
			_clientMap[completionKey]->AddMessage(message, size);

			return NETWORK_OK; // 성공적으로 메시지를 추가
		}
		else
		{
			return NETWORK_ERROR; // 잘못된 completionKey
		}
	}

	void* ClientManager::GetReceiveMessageFromClient(DWORD completionKey)
	{
		if (completionKey < _maxClient)
		{
			Network::MessageData* messageData = new Network::MessageData(completionKey, (char*)_clientMap[completionKey]->GetReceiveMessage());

			return messageData;
		}
		else
		{
			return nullptr; 
		}
	}

}