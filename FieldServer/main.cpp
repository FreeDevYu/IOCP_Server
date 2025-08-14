#pragma once
#include "FieldServer.h"
#include "../Network/NetworkConfig.h"

void DebugLog(const std::string& type, const std::string& message);

int main()
{
    auto config = Utility::LoadSettingFiles("fieldServer_config.json");

    Field::FieldServerDefine::Instance().SetOverlappedCount(config["OVERLAPPED_COUNT_MAX"].get<int>());
    Field::FieldServerDefine::Instance().SetMaxClient(config["CLIENT_CAPACITY"].get<int>());
        
    Field::FieldServerDefine::Instance().SetHeartBeatInterval(config["HEARTBEAT_INTERVAL"].get<int>());
    Field::FieldServerDefine::Instance().SetHeartBeatTimeout(config["HEARTBEAT_TIMEOUT"].get<int>());
    Field::FieldServerDefine::Instance().SetHeartBeatMaxCount(config["HEARTBEAT_TIMEOUT_CHANCE"].get<int>());
   
    Field::FieldServerDefine::Instance().SetRegisterWaitTime(config["REGISTER_TIMEOUT"].get<int>());

    Field::FieldServer fieldServer;

    fieldServer.SetDebugLogCallback
    (
        [](const std::string& type, const std::string& message) {
			DebugLog(type, message);
        }
    );

    std::thread serverPoewrOnThread(
        [&fieldServer]() {
            fieldServer.PowerOnSequence();
        }
    );
    serverPoewrOnThread.detach();

    while (true)
    {

    }
}

void DebugLog(const std::string& type, const std::string& message)
{
    std::cout << "[" << type << "] " << message << std::endl;
}