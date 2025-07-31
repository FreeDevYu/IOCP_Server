#pragma once
#include "../Network/NetworkBaseServer.h"

class ServerManager : public Network::NetworkBaseServer
{
public:
	ServerManager();
	~ServerManager() override;

public:
	int	WorkProcess() override;
	int	AcceptProcess() override;
	int	UpdateProcess() override;

};

