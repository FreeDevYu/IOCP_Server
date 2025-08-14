#pragma once
#include "../Network/NetworkBaseServer.h"
#include "FieldServerDefine.h"
#include "Player.h"

#include "oneTBB/include/oneapi/tbb/concurrent_queue.h"
#include "oneTBB/include/oneapi/tbb/concurrent_map.h"

namespace Field
{
	class FieldServer : public Network::NetworkBaseServer
	{
	public:
		FieldServer();
		~FieldServer() override;

	public:
		std::function<void(const std::string&, const std::string&)> _debugLogCallback;
		void SetDebugLogCallback(std::function<void(const std::string&, const std::string&)> callback);

	private:
		tbb::concurrent_map<DWORD, Field::Player*> _playerMap;

	public:
		int PowerOnSequence() override; // 서버 시작 시 초기화 작업을 수행합니다.

		int	WorkProcess() override;
		int	AcceptProcess() override;
		int	UpdateProcess() override;

	protected:
		int DisconnectClient(DWORD completionKey) override;

	private:
		tbb::concurrent_queue<std::shared_ptr<Network::MessageData>> _messageQueue;
		void RecvMessageProcess();
		void ReadMessage(std::shared_ptr<Network::MessageData> messageData);
	};
}