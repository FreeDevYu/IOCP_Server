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
		DWORD _completionKey; // �÷��̾��� ���� Ű
		DWORD _registerTime; // ��� �ð�
		std::function<void(const std::string&, const std::string&)> _debugLogCallback;

		//HeartBeat ���� ����
		int _timeOutCount;
		unsigned long long _lastRequestTime; // ������ HEARTBEAT ���� �ð�
		unsigned long long _lastResponseTime; // ������ HEARTBEAT ���� �ð�

	};
}