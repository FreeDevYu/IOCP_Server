#include "ServerManager.h"

namespace Manager
{
	void ServerManager::PlayerOnlineCheck(DWORD currentTime)
	{
		int offlineCheck = 0;
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
			if (!player->IsHeartbeatTarget())
				continue;

			offlineCheck = player->CheckKickoutTarget(currentTime);
			if(offlineCheck == true)
			{
				// �÷��̾ �������� ���·� ����Ǿ��� �� ó��
				DebugLog(Debug::DEBUG_LOG, std::format("Player is offline: {}", player->GetServerName()));

				//_clientManager->RemoveClient(player->GetCompletionKey());
				//delete player; // �޸� ����
				//_playerMap.erase(it); // �÷��̾� �ʿ��� ����
			}
			
		}

		DebugLog(Debug::DEBUG_LOG, "PlayerOnlineCheck Complete");
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

			if (!player->IsHeartbeatTarget())
				continue;

			player->SaveRequestHearbeatTime();
			_clientManager->AddMessageToClient(player->GetCompletionKey(), builder.GetBufferPointer(), builder.GetSize());
		}

		DebugLog(Debug::DEBUG_LOG, "ProcessHeartBeat");
	}
	
	bool Player::IsHeartbeatTarget() const
	{
		return _serverStatus != ServerStatus::OFFLINE && _serverStatus != ServerStatus::NOT_REGIST;
	}

	bool Player::CheckKickoutTarget(DWORD currentTime)
	{
		if(_serverStatus == ServerStatus::NOT_REGIST && _registerTime + Manager::ServerManagerDefine::Instance().GetRegisterWaitTime() < currentTime)
		{
			// ��� ��� �ð��� �ʰ��� ���
			_serverStatus = ServerStatus::OFFLINE; // ���� ���¸� OFFLINE���� ����
			return true;
		}

		// ��Ʈ��Ʈ Ÿ�Ӿƿ� �߻�
		if (_serverStatus == ServerStatus::REQUEST && _lastRequestTime + Manager::ServerManagerDefine::Instance().GetHeartBeatTimeout() < currentTime)
		{
			_timeOutCount++;
			_serverStatus = ServerStatus::TIMEOUT; // ���� ���¸� OFFLINE���� ����

			if (_timeOutCount > Manager::ServerManagerDefine::Instance().GetHeartBeatMaxCount())
			{
				_serverStatus = ServerStatus::OFFLINE; // �ִ� Ÿ�Ӿƿ� Ƚ�� �ʰ� �� OFFLINE ���·� ����
				return true;
			}
		}

		return false;
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