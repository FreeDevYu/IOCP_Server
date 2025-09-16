#include "Player.h"

namespace Field
{
	Player::Player()
		: _completionKey(0)
		, _onlineStatus(OnlineStatus::NOT_REGIST)
		, _playerID("")
		, _lastResponseTime(0)
		, _debugLogCallback(nullptr)
	{
	}

	Player::~Player()
	{
		_completionKey = 0;
		_onlineStatus = OnlineStatus::OFFLINE;
		_playerID = "";
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

		_moveSpeed = 5.0f; // 이동 속도 설정 (예: 초당 5 유닛)
		_rotation = 0.0f; // 초기 회전 각도 설정
		_xPosition = 0.0f;
		_yPosition = 0.0f;
		_zPosition = 0.0f;
	}

	void Player::Register(const std::string& playerID)
	{
		_playerID = playerID;
		_onlineStatus = OnlineStatus::REQUEST; // 서버 등록 요청 상태로 변경
		_lastResponseTime = GetTickCount(); // 현재 시간으로 초기화
	}

	DWORD Player::GetCompletionKey() const
	{
		return _completionKey;
	}

	std::string Player::GetPlayerID() const
	{
		return _playerID;
	}

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

	void Player::MovePosition(int8_t direction, float duration, float& angleDeg)
	{
		float moveDistance = _moveSpeed * duration;

		// 방향별 고정 각도 (도 단위)
		switch (direction)
		{
		case 1: angleDeg = 0.0f;    break;
		case 2: angleDeg = 180.0f;  break;
		case 3: angleDeg = -90.0f;  break;
		case 4: angleDeg = 90.0f;   break;
		case 5: angleDeg = -45.0f;  break;
		case 6: angleDeg = 45.0f;   break;
		case 7: angleDeg = -135.0f; break;
		case 8: angleDeg = 135.0f;  break;
		case 0:
		default: return; // 움직이지 않음
		}

		// 각도를 라디안으로 변환
		float radians = angleDeg * M_PI / 180.0f;

		// 방향 벡터 계산
		float dirX = sin(radians);
		float dirZ = cos(radians);

		// 위치 갱신
		_xPosition += dirX * moveDistance;
		_zPosition += dirZ * moveDistance;
	}
}