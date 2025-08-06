#include "ServerManager.h"

namespace Manager
{
	void ServerManager::SettingExternalCommands()
	{
		_commandMap.insert({ "insert_server_ip", std::bind(&ServerManager::AddServerIP, this, std::placeholders::_1) });
	

	}

	void ServerManager::ReceiveExternalCommand(const std::string& command)
	{
		// �ܺ� ��ɾ ó���ϴ� ������ �����մϴ�.
		// ���� ���, ���� ���� Ȯ��, Ŭ���̾�Ʈ ���� ���� �۾��� ������ �� �ֽ��ϴ�.

		// ��ɾ �������� �и��Ͽ� ��ɾ�� ���ڸ� �����մϴ�.
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
			param = "";  // ���ڰ� ���� ���
		}

		if(_commandMap.find(cmd) != _commandMap.end())
		{
			// �ش� ��ɾ ��ϵǾ� �ִ� ���
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
}