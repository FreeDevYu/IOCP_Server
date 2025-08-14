#pragma once
#include<functional>
#include <WinSock2.h>

namespace Field
{
	class Player
	{
	public:
		Player();
		~Player();

	public:
		void Initialize(const DWORD completionKey, const DWORD registerTime, std::function<void(const std::string&, const std::string&)> debugLogCallback);

		void Register();
		DWORD GetCompletionKey() const;

	private:
		DWORD _completionKey; // 플레이어의 고유 키
		DWORD _registerTime; // 등록 시간
		std::function<void(const std::string&, const std::string&)> _debugLogCallback;

		//HeartBeat 관련 변수
		int _timeOutCount;
		unsigned long long _lastRequestTime; // 마지막 HEARTBEAT 응답 시간
		unsigned long long _lastResponseTime; // 마지막 HEARTBEAT 응답 시간

	};
}