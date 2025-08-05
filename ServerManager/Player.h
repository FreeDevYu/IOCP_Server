#pragma once
#include <string>
#include <atomic>
#include <WinSock2.h>

namespace Manager
{
	enum ServerStatus
	{
		ONLINE = -1,

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
		void Initialize(const std::string& serverName, const DWORD completionKey);
		DWORD GetCompletionKey() const;

	private:
		DWORD _completionKey; // �÷��̾��� ���� Ű
		ServerStatus _serverStatus; // ���� ����
		std::string _serverName; // ���� �̸�

		int _timeOutCount;
		DWORD _lastRequestTime; // ������ HEARTBEAT ���� �ð�
		DWORD _lastResponseTime; // ������ HEARTBEAT ���� �ð�


	public:

		bool IsOnline() const;
		int HearbeatCheck(DWORD currentTime);
		void SaveRequestHearbeatTime();
		void ResponseHeartBeat();
		
	};



}