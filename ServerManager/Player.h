#pragma once
#include <string>
#include <atomic>
#include <WinSock2.h>


namespace Manager
{
	enum ServerStatus
	{
		DEFAULT = -1,

		ONLINE = 0, // ������ �¶��� ����
		OFFLINE, // ������ �������� ����

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
		DWORD _completionKey; // �÷��̾��� ���� Ű
		ServerStatus _serverStatus; // ���� ����
		std::string _serverName; // ���� �̸�
		std::atomic<DWORD> _lastResponseTime; // ������ HEARTBEAT ���� �ð�

	};



}