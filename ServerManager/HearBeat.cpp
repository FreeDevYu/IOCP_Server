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
			if (!player->IsOnline())
				continue;

			offlineCheck = player->HearbeatCheck(currentTime);
			if(offlineCheck == ServerStatus::OFFLINE)
			{
				// 플레이어가 오프라인 상태로 변경되었을 때 처리
				DebugLog(Debug::DEBUG_LOG, std::format("Player is offline: {}", player->GetServerName()));

				//_clientManager->RemoveClient(player->GetCompletionKey());
				//delete player; // 메모리 해제
				//_playerMap.erase(it); // 플레이어 맵에서 제거
			}
			
		}

		DebugLog(Debug::DEBUG_LOG, "PlayerOnlineCheck");
	}

	void ServerManager::ProcessHeartBeat()
	{
		int size = _playerMap.size();
		flatbuffers::FlatBufferBuilder builder;// while문 안에서 생성된 builder는 while문종료시 삭제된다.
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

		DebugLog(Debug::DEBUG_LOG, "ProcessHeartBeat");
	}
	
	bool Player::IsOnline() const
	{
		return _serverStatus != ServerStatus::OFFLINE;
	}

	int Player::HearbeatCheck(DWORD currentTime)
	{
		// 하트비트 타임아웃 발생
		if (_serverStatus == ServerStatus::REQUEST && _lastRequestTime + Manager::ServerManagerDefine::Instance().GetHeartBeatTimeout() < currentTime)
		{
			_timeOutCount++;
			_serverStatus = ServerStatus::TIMEOUT; // 서버 상태를 OFFLINE으로 변경

			if (_timeOutCount > Manager::ServerManagerDefine::Instance().GetHeartBeatMaxCount())
			{
				_serverStatus = ServerStatus::OFFLINE; // 최대 타임아웃 횟수 초과 시 OFFLINE 상태로 변경
				//오프라인처리
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
			// 이미 오프라인 상태인 경우, 아무 작업도 하지 않음
			return;
		}

		_serverStatus = ServerStatus::RESPONSE;
		_lastResponseTime = GetTickCount();;
	}
} 