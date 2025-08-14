#include "Player.h"

namespace Field
{

	Player::Player()
		: _completionKey(0)
		, _registerTime(0)
		, _timeOutCount(0)
		, _lastRequestTime(0)
		, _lastResponseTime(0)
		, _debugLogCallback(nullptr)
	{
	}

	Player::~Player()
	{

	}

	void Player::Initialize(const DWORD completionKey, const DWORD registerTime, std::function<void(const std::string&, const std::string&)> debugLogCallback)
	{
		_completionKey = completionKey;
		_registerTime = registerTime;
		_timeOutCount = 0;
		_debugLogCallback = debugLogCallback;
	}
	void Player::Register()
	{
		//DebugLog("INFO", "Player registered with completion key: " + std::to_string(_completionKey));
	}

	DWORD Player::GetCompletionKey() const
	{
		return _completionKey;
	}
}