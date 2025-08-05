#include "ServerManager.h"

namespace Manager
{
	void ServerManager::PlayerOnlineCheck(DWORD currentTime)
	{
		int size = _playerMap.size();
		for (int i = 0;i < size; ++i)
		{
			auto it = _playerMap.begin();
			std::advance(it, i);
			if (it == _playerMap.end())
				break;
			auto player = it->second;
			if (player == nullptr)
				continue;
			if (!player->IsOnline())
				continue;

			player->HearbeatCheck(currentTime);
		}
	}

	void ServerManager::ProcessHeartBeat()
	{
		int size = _playerMap.size();
		flatbuffers::FlatBufferBuilder builder;// while�� �ȿ��� ������ builder�� while������� �����ȴ�.
		builder.Finish(protocol::CreateREQUEST_HEARTBEAT(builder));

		for(int i=0;i< size; ++i)
		{
			auto it = _playerMap.begin();
			std::advance(it, i);
			if (it == _playerMap.end())
				break;
			auto player = it->second;
			if (player == nullptr)
				continue;

			if (!player->IsOnline())
				continue;

			player->SaveRequestHearbeatTime();
			_clientManager->AddMessageToClient(player->GetCompletionKey(), builder.GetBufferPointer(), builder.GetSize());
		}
	}
	
	bool Player::IsOnline() const
	{
		return _serverStatus != ServerStatus::OFFLINE;
	}

	int Player::HearbeatCheck(DWORD currentTime)
	{
		// ��Ʈ��Ʈ Ÿ�Ӿƿ� �߻�
		if (_serverStatus == ServerStatus::REQUEST && _lastRequestTime + Manager::ServerManagerDefine::Instance().GetHeartBeatTimeout() < currentTime)
		{
			_timeOutCount++;
			_serverStatus = ServerStatus::TIMEOUT; // ���� ���¸� OFFLINE���� ����

			if (_timeOutCount > Manager::ServerManagerDefine::Instance().GetHeartBeatMaxCount())
			{
				_serverStatus = ServerStatus::OFFLINE; // �ִ� Ÿ�Ӿƿ� Ƚ�� �ʰ� �� OFFLINE ���·� ����
				//��������ó��
			}
		}

		return _serverStatus;
	}

	void Player::SaveRequestHearbeatTime()
	{
		_serverStatus = ServerStatus::REQUEST; 
		_lastRequestTime = GetTickCount();;
	}

	void Player::ResponseHeartBeat()
	{
		if (_serverStatus == ServerStatus::OFFLINE)
		{
			// �̹� �������� ������ ���, �ƹ� �۾��� ���� ����
			return;
		}

		_serverStatus = ServerStatus::RESPONSE;
		_lastResponseTime = GetTickCount();;
	}
} 