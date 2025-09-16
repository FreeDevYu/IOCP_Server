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
		_onlineStatus = OnlineStatus::NOT_REGIST; // �ʱ� ���´� ONLINE�� ����
		_timeOutCount = 0;
		_debugLogCallback = debugLogCallback;

		_moveSpeed = 5.0f; // �̵� �ӵ� ���� (��: �ʴ� 5 ����)
		_rotation = 0.0f; // �ʱ� ȸ�� ���� ����
		_xPosition = 0.0f;
		_yPosition = 0.0f;
		_zPosition = 0.0f;
	}

	void Player::Register(const std::string& playerID)
	{
		_playerID = playerID;
		_onlineStatus = OnlineStatus::REQUEST; // ���� ��� ��û ���·� ����
		_lastResponseTime = GetTickCount(); // ���� �ð����� �ʱ�ȭ
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
			// ��� ��� �ð��� �ʰ��� ���
			_onlineStatus = OnlineStatus::OFFLINE; // ���� ���¸� OFFLINE���� ����
			return true;
		}

		// ��Ʈ��Ʈ Ÿ�Ӿƿ� �߻�
		if (_onlineStatus == OnlineStatus::REQUEST && _lastRequestTime + Field::FieldServerDefine::Instance().GetHeartBeatTimeout() < currentTime)
		{
			_timeOutCount++;
			_onlineStatus = OnlineStatus::TIMEOUT; // RESPONSE�� �ص� ��������

			//DebugLog("DEBUG_LOG", std::format("Player {} is in TIMEOUT state. Timeout count: {}", _serverName, _timeOutCount));
			if (_timeOutCount > Field::FieldServerDefine::Instance().GetHeartBeatMaxCount())
			{
				_onlineStatus = OnlineStatus::OFFLINE; // �ִ� Ÿ�Ӿƿ� Ƚ�� �ʰ� �� OFFLINE ���·� ����

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
			// �̹� �������� ������ ���, �ƹ� �۾��� ���� ����
			return;
		}

		_onlineStatus = OnlineStatus::RESPONSE;
		_lastResponseTime = GetTickCount64();;
		_timeOutCount = 0;
	}

	void Player::MovePosition(int8_t direction, float duration, float& angleDeg)
	{
		float moveDistance = _moveSpeed * duration;

		// ���⺰ ���� ���� (�� ����)
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
		default: return; // �������� ����
		}

		// ������ �������� ��ȯ
		float radians = angleDeg * M_PI / 180.0f;

		// ���� ���� ���
		float dirX = sin(radians);
		float dirZ = cos(radians);

		// ��ġ ����
		_xPosition += dirX * moveDistance;
		_zPosition += dirZ * moveDistance;
	}
}