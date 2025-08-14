#include "FieldServer.h"

namespace Field
{
	void FieldServer::SettingExternalCommands()
	{
		// �����Ŵ��� ����޼����� �߰��Ұ�.
	//	_commandMap.insert({ "port", std::bind(&FieldServer::GetServerPort, this, std::placeholders::_1) });
	//	_commandMap.insert({ "name", std::bind(&FieldServer::GetServerName, this, std::placeholders::_1) });
	//	_commandMap.insert({ "allow_ip", std::bind(&FieldServer::AddConnectPermissionIp, this, std::placeholders::_1) });
	
	}

	void FieldServer::ReceiveExternalCommand(std::string& input)
	{
		// �ܺ� ��ɾ ó���ϴ� ������ �����մϴ�.
		// ���� ���, ���� ���� Ȯ��, Ŭ���̾�Ʈ ���� ���� �۾��� ������ �� �ֽ��ϴ�.

		std::string command;
		std::vector<std::string> parameters;

		// '['�� ']' ���� ���ڿ� ����
		size_t start = input.find('[');
		size_t end = input.find(']');

		if (start == std::string::npos || end == std::string::npos || end <= start)
			return; // �߸��� ����

		command = input.substr(0, start);
		std::string content = input.substr(start + 1, end - start - 1);

		// ',' �������� ������
		std::stringstream ss(content);
		std::string parameter;
		while (std::getline(ss, parameter, ','))
		{
			//���� ����
			size_t first = parameter.find_first_not_of(" \t");
			size_t last = parameter.find_last_not_of(" \t");
			if (first != std::string::npos && last != std::string::npos)
				parameter = parameter.substr(first, last - first + 1);

			parameters.push_back(parameter);
		}


		if(_commandMap.find(command) != _commandMap.end())
		{
			// �ش� ��ɾ ��ϵǾ� �ִ� ���
			_commandMap[command](parameters);
		}
		else
		{
			DebugLog(Debug::DEBUG_ERROR, std::format("ReceiveExternalCommand: Unknown command '{}'", command));
		}
	}

//	//Init
//	void FieldServer::GetServerPort(std::vector<std::string> parameter)
//	{
//		std::string port = parameter[0];
//		if(_serverOn)
//		{
//			DebugLog(Debug::DEBUG_LOG, std::format("GetServerPort: Current Playing server Port: {}", port));
//			return;
//		}
//
//		try
//		{
//			_serverPort = std::stoi(port);
//			DebugLog(Debug::DEBUG_LOG, std::format("GetServerPort: Setting port to {} and powering on.", port));
//		}
//		catch (const std::exception& e)
//		{
//			_serverPort = 0; // �ʱ�ȭ
//			DebugLog(Debug::DEBUG_ERROR, std::format("GetServerPort: Invalid port input '{}'. Error: {}", port, e.what()));
//		}
//		
//		//finally
//		::SetEvent(_powerOnEvent);  // Windows API ����̶��
//	}
//
//	//Init
//	void FieldServer::GetServerName(std::vector<std::string> parameter)
//	{
//		std::string serverName = parameter[0];
//		if(_serverOn)
//		{
//			DebugLog(Debug::DEBUG_LOG, std::format("GetServerName: Current Playing server Name: {}", serverName));
//			return;
//		}
//
//		try
//		{
//			_hostName = serverName;
//			DebugLog(Debug::DEBUG_LOG, std::format("GetServerName: Setting server name to '{}'.", serverName));
//		}
//		catch (const std::exception& e)
//		{
//			_hostName = "";
//			DebugLog(Debug::DEBUG_ERROR, std::format("GetServerName: Invalid server name input '{}'. Error: {}", serverName, e.what()));
//		}
//
//		//finally
//		::SetEvent(_powerOnEvent);  // Windows API ����̶��
//	}

//void FieldServer::AddConnectPermissionIp(std::vector<std::string> parameter)
//{
//	std::string ip = parameter[0];
//	if (ip.empty())
//	{
//		DebugLog(Debug::DEBUG_ERROR, "AddServerIP: Invalid IP address.");
//		return;
//	}
//
//	if (_serverIpSet.find(ip) != _serverIpSet.end())
//	{
//		DebugLog(Debug::DEBUG_WARNING, std::format("AddServerIP: IP {} already exists.", ip));
//		return;
//	}
//
//	_serverIpSet.insert(ip);
//	DebugLog(Debug::DEBUG_LOG, std::format("AddServerIP: IP {} added successfully.", ip));
//}
//
//bool FieldServer::CheckServerIP(const std::string& ip) const
//{
//	return _serverIpSet.find(ip) != _serverIpSet.end();
//}
}