#pragma once

#include<functional>
#include <WinSock2.h>
#include <string>
#include <cmath>

#include "FieldServerDefine.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace Field
{
	enum OnlineStatus
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
		std::string GetPlayerID() const;

	private:
		DWORD _completionKey; // 플레이어의 고유 키
		OnlineStatus _onlineStatus; // 서버 상태
		std::string _playerID; // 서버 이름

		DWORD _registerTime; // 서버 등록 시간

		int _timeOutCount;
		unsigned long long _lastRequestTime; // 마지막 HEARTBEAT 응답 시간
		unsigned long long _lastResponseTime; // 마지막 HEARTBEAT 응답 시간

		std::function<void(const std::string&, const std::string&)> _debugLogCallback;

		float _moveSpeed;
		float _rotation; // Yaw 각도 (0~360)

	public://임시
		float _xPosition = 0.0f;
		float _yPosition = 0.0f;
		float _zPosition = 0.0f;
		float GetMoveSpeed() const { return _moveSpeed; }

	public:
		bool IsHeartbeatTarget() const;
		bool CheckKickoutTarget(unsigned long long currentTime);
		void SaveRequestHearbeatTime();
		void SaveResponseHeartBeat();
		void MovePosition(int8_t direction, float duration, float& angleDeg);

	private:
		void DebugLog(const std::string& type, const std::string& message);

	};
}