#include "Player.h"


namespace Manager
{
	Player::Player()
		: _completionKey(0)
		, _serverStatus(ServerStatus::NOT_REGIST)
		, _lastResponseTime(0)
	{
	}

	Player::~Player()
	{
		_completionKey = 0;
		_serverStatus = ServerStatus::OFFLINE;
		_lastResponseTime = 0;
	}

	void Player::Initialize(const DWORD completionKey, const DWORD registerTime)
	{
		_completionKey = completionKey;
		_registerTime = registerTime;
		_serverStatus = ServerStatus::NOT_REGIST; // �ʱ� ���´� ONLINE�� ����
		_timeOutCount = 0;
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
}