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

		REQUEST = 0, // ��û ��� ����
		RESPONSE = 1, // ���� ��� ����
		TIMEOUT = 2, // Ÿ�Ӿƿ� ����
		OFFLINE = 3, // �������� ����

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
		DWORD _completionKey; // �÷��̾��� ���� Ű
		ServerStatus _serverStatus; // ���� ����
		std::string _serverName; // ���� �̸�

		DWORD _registerTime; // ���� ��� �ð�

		int _timeOutCount;
		unsigned long long _lastRequestTime; // ������ HEARTBEAT ���� �ð�
		unsigned long long _lastResponseTime; // ������ HEARTBEAT ���� �ð�

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