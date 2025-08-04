#pragma once
#include <string>
#include <atomic>
#include <WinSock2.h>


namespace Manager
{
	enum ServerStatus
	{
		DEFAULT = -1,

		ONLINE = 0, // 서버가 온라인 상태
		OFFLINE, // 서버가 오프라인 상태

		END
	};

	class Player
	{
	public:
		Player();
		~Player();

	public:
		void Initialize(const std::string& serverName, const DWORD completionKey);

	private:
		DWORD _completionKey; // 플레이어의 고유 키
		ServerStatus _serverStatus; // 서버 상태
		std::string _serverName; // 서버 이름
		std::atomic<DWORD> _lastResponseTime; // 마지막 HEARTBEAT 응답 시간

	};



}