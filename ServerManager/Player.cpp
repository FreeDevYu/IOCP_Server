#include "Player.h"


namespace Manager
{
	Player::Player()
		: _completionKey(0)
		, _serverStatus(ServerStatus::ONLINE)
		, _lastResponseTime(0)
	{
	}

	Player::~Player()
	{
		_completionKey = 0;
		_serverStatus = ServerStatus::OFFLINE;
		_lastResponseTime = 0;
	}

	void Player::Initialize(const std::string& serverName, const DWORD completionKey)
	{
		_completionKey = completionKey;
		_serverName = serverName;
		_serverStatus = ServerStatus::ONLINE; // �ʱ� ���´� ONLINE�� ����
		_lastResponseTime = GetTickCount(); // ���� �ð����� �ʱ�ȭ
	}

	DWORD Player::GetCompletionKey() const
	{
		return _completionKey;
	}
}