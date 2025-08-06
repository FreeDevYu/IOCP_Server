#pragma once
#include <string>
#include <atomic>
#include <WinSock2.h>

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
		void Initialize(const DWORD completionKey, const DWORD registerTime);
		void Register(const std::string& serverName);
		DWORD GetCompletionKey() const;
		std::string GetServerName() const;
	private:
		DWORD _completionKey; // �÷��̾��� ���� Ű
		ServerStatus _serverStatus; // ���� ����
		std::string _serverName; // ���� �̸�

		DWORD _registerTime; // ���� ��� �ð�

		int _timeOutCount;
		DWORD _lastRequestTime; // ������ HEARTBEAT ���� �ð�
		DWORD _lastResponseTime; // ������ HEARTBEAT ���� �ð�


	public:

		bool IsHeartbeatTarget() const;
		bool CheckKickoutTarget(DWORD currentTime);
		void SaveRequestHearbeatTime();
		void ResponseHeartBeat();
		
	};



}