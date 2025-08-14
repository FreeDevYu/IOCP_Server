#include "Player.h"

namespace Field
{
	Player::Player()
		: _completionKey(0)
		, _onlineStatus(OnlineStatus::NOT_REGIST)
		, _lastResponseTime(0)
		, _debugLogCallback(nullptr)
	{
	}

	Player::~Player()
	{
		_completionKey = 0;
		_onlineStatus = OnlineStatus::OFFLINE;
		_lastResponseTime = 0;
		_debugLogCallback = nullptr;
	}

	void Player::Initialize(const DWORD completionKey, const DWORD registerTime, std::function<void(const std::string&, const std::string&)> debugLogCallback)
	{
		_completionKey = completionKey;
		_registerTime = registerTime;
		_onlineStatus = OnlineStatus::NOT_REGIST; // 초기 상태는 ONLINE로 설정
		_timeOutCount = 0;
		_debugLogCallback = debugLogCallback;
	}

	//void Player::Register(const std::string& serverName)
	//{
	//	_serverName = serverName;
	//	_onlineStatus = OnlineStatus::REQUEST; // 서버 등록 요청 상태로 변경
	//	_lastResponseTime = GetTickCount(); // 현재 시간으로 초기화
	//}

	DWORD Player::GetCompletionKey() const
	{
		return _completionKey;
	}

//	std::string Player::GetServerName() const
//	{
//		return _serverName;
//	}

	void Player::DebugLog(const std::string& type, const std::string& message)
	{
		if (_debugLogCallback)
		{
			_debugLogCallback(type, message);
		}
	}

	bool Player::IsHeartbeatTarget() const
	{
		return _onlineStatus != OnlineStatus::OFFLINE && _onlineStatus != OnlineStatus::NOT_REGIST;
	}

	bool Player :: CheckKickoutTarget(unsigned long long currentTime)
	{
		if (_onlineStatus == OnlineStatus::NOT_REGIST && _registerTime + Field::FieldServerDefine::Instance().GetRegisterWaitTime() < currentTime)
		{
			// 등록 대기 시간이 초과된 경우
			_onlineStatus = OnlineStatus::OFFLINE; // 서버 상태를 OFFLINE으로 변경
			return true;
		}

		// 하트비트 타임아웃 발생
		if (_onlineStatus == OnlineStatus::REQUEST && _lastRequestTime + Field::FieldServerDefine::Instance().GetHeartBeatTimeout() < currentTime)
		{
			_timeOutCount++;
			_onlineStatus = OnlineStatus::TIMEOUT; // RESPONSE로 해도 괜찮을듯

			//DebugLog("DEBUG_LOG", std::format("Player {} is in TIMEOUT state. Timeout count: {}", _serverName, _timeOutCount));
			if (_timeOutCount > Field::FieldServerDefine::Instance().GetHeartBeatMaxCount())
			{
				_onlineStatus = OnlineStatus::OFFLINE; // 최대 타임아웃 횟수 초과 시 OFFLINE 상태로 변경

				return true;
			}
		}

		return false;
	}
	void Player::SaveRequestHearbeatTime()
	{
		_onlineStatus = OnlineStatus::REQUEST;
		_lastRequestTime = GetTickCount64();;
	}

	void Player::SaveResponseHeartBeat()
	{
		if (_onlineStatus == OnlineStatus::OFFLINE)
		{
			// 이미 오프라인 상태인 경우, 아무 작업도 하지 않음
			return;
		}

		_onlineStatus = OnlineStatus::RESPONSE;
		_lastResponseTime = GetTickCount64();;
		_timeOutCount = 0;
	}

}