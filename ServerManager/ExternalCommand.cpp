#include "ServerManager.h"

namespace Manager
{
	void ServerManager::SettingExternalCommands()
	{
		_commandMap.insert({ "insert_server_ip", std::bind(&ServerManager::AddServerIP, this, std::placeholders::_1) });
	

	}

	void ServerManager::ReceiveExternalCommand(const std::string& command)
	{
		// 외부 명령어를 처리하는 로직을 구현합니다.
		// 예를 들어, 서버 상태 확인, 클라이언트 관리 등의 작업을 수행할 수 있습니다.

		// 명령어를 공백으로 분리하여 명령어와 인자를 구분합니다.
		size_t spacePos = command.find(' ');
		std::string cmd;
		std::string param;

		if (spacePos != std::string::npos)
		{
			cmd = command.substr(0, spacePos);
			param = command.substr(spacePos + 1);
		}
		else
		{
			cmd = command;
			param = "";  // 인자가 없는 경우
		}

		if(_commandMap.find(cmd) != _commandMap.end())
		{
			// 해당 명령어가 등록되어 있는 경우
			_commandMap[cmd](param);
		}
		else
		{
			DebugLog(Debug::DEBUG_ERROR, std::format("ReceiveExternalCommand: Unknown command '{}'", cmd));
		}
	}

	void ServerManager::AddServerIP(const std::string& ip)
	{
		if (ip.empty())
		{
			DebugLog(Debug::DEBUG_ERROR, "AddServerIP: Invalid IP address.");
			return;
		}

		if (_serverIpSet.find(ip) != _serverIpSet.end())
		{
			DebugLog(Debug::DEBUG_WARNING, std::format("AddServerIP: IP {} already exists.", ip));
			return;
		}

		_serverIpSet.insert(ip);
		DebugLog(Debug::DEBUG_LOG, std::format("AddServerIP: IP {} added successfully.", ip));
	}

	bool ServerManager::CheckServerIP(const std::string& ip) const
	{
		return _serverIpSet.find(ip) != _serverIpSet.end();
	}
}