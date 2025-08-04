#include "Player.h"

namespace Manager
{
	Player::Player()
		: _completionKey(0)
		, _serverStatus(ServerStatus::DEFAULT)
		, _lastResponseTime(0)
	{
	}

	Player::~Player()
	{
		_completionKey = 0;
		_serverStatus = ServerStatus::DEFAULT;
		_lastResponseTime = 0;
	}

	void Player::Initialize(const std::string& serverName, const DWORD completionKey)
	{
		_completionKey = completionKey;
		_serverName = serverName;
		_serverStatus = ServerStatus::ONLINE; // 초기 상태는 ONLINE
		_lastResponseTime = GetTickCount(); // 현재 시간으로 초기화
	}

	//DWORD Player::GetCompletionKey() const
	//{
	//	return _completionKey;
	//}
}