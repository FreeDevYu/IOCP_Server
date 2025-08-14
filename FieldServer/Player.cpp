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
		_onlineStatus = OnlineStatus::NOT_REGIST; // �ʱ� ���´� ONLINE�� ����
		_timeOutCount = 0;
		_debugLogCallback = debugLogCallback;
	}

	//void Player::Register(const std::string& serverName)
	//{
	//	_serverName = serverName;
	//	_onlineStatus = OnlineStatus::REQUEST; // ���� ��� ��û ���·� ����
	//	_lastResponseTime = GetTickCount(); // ���� �ð����� �ʱ�ȭ
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

}