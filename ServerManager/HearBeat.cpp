#include "ServerManager.h"

namespace Manager
{
	void ServerManager::PlayerOnlineCheck(unsigned long long currentTime)
	{
		bool offlineCheck = false;
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
				SendTelegramMessage(std::format("Server {} is OUT.", player->GetServerName()));
			}
			
		}

		//DebugLog(Debug::DEBUG_LOG, "PlayerOnlineCheck Complete"); -> �ʹ����� ȣ��Ǿ� �ּ�ó��
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

			Network::MessageHeader header(builder.GetSize(), protocol::MESSAGETYPE::MESSAGETYPE_REQUEST_HEARTBEAT);
			std::shared_ptr<Network::MessageData> messageData = std::make_shared<Network::MessageData>(
				player->GetCompletionKey(),
				header,
				(char*)builder.GetBufferPointer()
			);

			SendMessageToClient(player->GetCompletionKey(), messageData);
			DebugLog(Debug::DEBUG_LOG, std::format("SEND [REQUEST HEARTBEAT] To Player {}.", player->GetServerName()));
		}

		DebugLog(Debug::DEBUG_LOG, "ProcessHeartBeat");
	}
	
	bool Player::IsHeartbeatTarget() const
	{
		return _serverStatus != ServerStatus::OFFLINE && _serverStatus != ServerStatus::NOT_REGIST;
	}

	bool Player::CheckKickoutTarget(unsigned long long currentTime)
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
			_serverStatus = ServerStatus::TIMEOUT; // RESPONSE�� �ص� ��������

			//Manager::ServerManager::DebugLog(Debug::DEBUG_LOG, std::format("Player {} is in TIMEOUT state. Timeout count: {}", _serverName, _timeOutCount));
			DebugLog("DEBUG_LOG", std::format("Player {} is in TIMEOUT state. Timeout count: {}", _serverName, _timeOutCount));
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
		_lastRequestTime = GetTickCount64();;
	}

	void Player::ResponseHeartBeat()
	{
		if (_serverStatus == ServerStatus::OFFLINE)
		{
			// �̹� �������� ������ ���, �ƹ� �۾��� ���� ����
			return;
		}

		_serverStatus = ServerStatus::RESPONSE;
		_lastResponseTime = GetTickCount64();;
		_timeOutCount = 0;
	}
} 