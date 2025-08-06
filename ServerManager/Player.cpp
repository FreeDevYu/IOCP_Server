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
		_serverStatus = ServerStatus::NOT_REGIST; // 초기 상태는 ONLINE로 설정
	}

	void Player::Register(const std::string& serverName)
	{
		_serverName = serverName;
		_serverStatus = ServerStatus::REQUEST; // 서버 등록 요청 상태로 변경
		_lastResponseTime = GetTickCount(); // 현재 시간으로 초기화
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