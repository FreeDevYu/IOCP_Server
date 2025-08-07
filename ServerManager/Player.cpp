#include "Player.h"


namespace Manager
{
	Player::Player()
		: _completionKey(0)
		, _serverStatus(ServerStatus::NOT_REGIST)
		, _lastResponseTime(0)
		, _debugLogCallback(nullptr)
	{
	}

	Player::~Player()
	{
		_completionKey = 0;
		_serverStatus = ServerStatus::OFFLINE;
		_lastResponseTime = 0;
		_debugLogCallback = nullptr;
	}

	void Player::Initialize(const DWORD completionKey, const DWORD registerTime, std::function<void(const std::string&, const std::string&)> debugLogCallback)
	{
		_completionKey = completionKey;
		_registerTime = registerTime;
		_serverStatus = ServerStatus::NOT_REGIST; // �ʱ� ���´� ONLINE�� ����
		_timeOutCount = 0;
		_debugLogCallback = debugLogCallback;
	}

	void Player::Register(const std::string& serverName)
	{
		_serverName = serverName;
		_serverStatus = ServerStatus::REQUEST; // ���� ��� ��û ���·� ����
		_lastResponseTime = GetTickCount(); // ���� �ð����� �ʱ�ȭ
	}

	DWORD Player::GetCompletionKey() const
	{
		return _completionKey;
	}

	std::string Player::GetServerName() const
	{
		return _serverName;
	}

	void Player::DebugLog(const std::string& type, const std::string& message)
	{
		if (_debugLogCallback)
		{
			_debugLogCallback(type, message);
		}
	}
}