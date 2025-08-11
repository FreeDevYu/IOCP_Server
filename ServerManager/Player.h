#pragma once
#include <string>
#include <atomic>
#include <WinSock2.h>
#include <functional>

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
		void Initialize(const DWORD completionKey, const DWORD registerTime, std::function<void(const std::string&, const std::string&)> debugLogCallback);
		void Register(const std::string& serverName);
		DWORD GetCompletionKey() const;
		std::string GetServerName() const;
	private:
		DWORD _completionKey; // 플레이어의 고유 키
		ServerStatus _serverStatus; // 서버 상태
		std::string _serverName; // 서버 이름

		DWORD _registerTime; // 서버 등록 시간

		int _timeOutCount;
		unsigned long long _lastRequestTime; // 마지막 HEARTBEAT 응답 시간
		unsigned long long _lastResponseTime; // 마지막 HEARTBEAT 응답 시간

		std::function<void(const std::string&, const std::string&)> _debugLogCallback;
	public:

		bool IsHeartbeatTarget() const;
		bool CheckKickoutTarget(unsigned long long currentTime);
		void SaveRequestHearbeatTime();
		void ResponseHeartBeat();
		
	private:
		void DebugLog(const std::string& type, const std::string& message);
		
	};



}