#pragma once
#include "../Network/NetworkBaseServer.h"
#include "oneTBB/include/oneapi/tbb/concurrent_queue.h"

class ServerManager : public Network::NetworkBaseServer
{
public:
	ServerManager();
	~ServerManager() override;

public:
	int	WorkProcess() override;
	int	AcceptProcess() override;
	int	UpdateProcess() override;

private:
	tbb::concurrent_queue<void*> _messageQueue;
	void RecvMessageProcess();
	void ReadMessage(void* message);



};

