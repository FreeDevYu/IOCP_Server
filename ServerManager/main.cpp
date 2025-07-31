#pragma once
#include "ServerManager.h"
#include "../Network/NetworkConfig.h"

//#define CONFGINTEST

int main()
{
#ifdef CONFGINTEST
	Utility::CreateConfig("servermanager_config.json");
#endif
	
	auto config = Utility::LoadSettingFiles("servermanager_config.json");

	ServerManager serverManager;
	serverManager.Initialize(
		new Network::BaseClientManager(),
		config["SERVER_PORT"].get<int>(),
		config["IP"].get<std::string>(),
		config["OVERLAPPED_COUNT_MAX"].get<int>(),
		config["CLIENT_CAPACITY"].get<int>()
	);

	serverManager.StartIOCP();
	serverManager.StartWorkThreads();
	serverManager.StartListenThread();
	serverManager.SetUpdateFrame(60); // FPS ¼³Á¤
	serverManager.StartUpdateThread();

	while (true)
	{

	}

	return 0;
}