#pragma once
#include <string>
#include <atomic>
#include <WinSock2.h>

namespace Manager
{
	enum ServerStatus
	{
		ONLINE = -1,

		REQUEST = 0, // 요청 대기 상태
		RESPONSE = 1, // 응답 대기 상태
		TIMEOUT = 2, // 타임아웃 상태
		OFFLINE = 3, // 오프라인 상태

		END
	};

	class Player
	{
	public:
		Player();
		~Player();

	public:
		void Initialize(const std::string& serverName, const DWORD completionKey);
		DWORD GetCompletionKey() const;

	private:
		DWORD _completionKey; // 플레이어의 고유 키
		ServerStatus _serverStatus; // 서버 상태
		std::string _serverName; // 서버 이름

		int _timeOutCount;
		DWORD _lastRequestTime; // 마지막 HEARTBEAT 응답 시간
		DWORD _lastResponseTime; // 마지막 HEARTBEAT 응답 시간


	public:

		bool IsOnline() const;
		int HearbeatCheck(DWORD currentTime);
		void SaveRequestHearbeatTime();
		void ResponseHeartBeat();
		
	};



}