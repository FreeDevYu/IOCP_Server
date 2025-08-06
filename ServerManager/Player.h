#pragma once
#include <string>
#include <atomic>
#include <WinSock2.h>

namespace Manager
{
	enum ServerStatus
	{
		NOT_REGIST = -1,

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
		void Initialize(const DWORD completionKey, const DWORD registerTime);
		void Register(const std::string& serverName);
		DWORD GetCompletionKey() const;
		std::string GetServerName() const;
	private:
		DWORD _completionKey; // 플레이어의 고유 키
		ServerStatus _serverStatus; // 서버 상태
		std::string _serverName; // 서버 이름

		DWORD _registerTime; // 서버 등록 시간

		int _timeOutCount;
		DWORD _lastRequestTime; // 마지막 HEARTBEAT 응답 시간
		DWORD _lastResponseTime; // 마지막 HEARTBEAT 응답 시간


	public:

		bool IsHeartbeatTarget() const;
		bool CheckKickoutTarget(DWORD currentTime);
		void SaveRequestHearbeatTime();
		void ResponseHeartBeat();
		
	};



}