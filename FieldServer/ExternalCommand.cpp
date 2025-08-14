#include "FieldServer.h"

namespace Field
{
	void FieldServer::SettingExternalCommands()
	{
		// 서버매니저 종료메세지도 추가할것.
	//	_commandMap.insert({ "port", std::bind(&FieldServer::GetServerPort, this, std::placeholders::_1) });
	//	_commandMap.insert({ "name", std::bind(&FieldServer::GetServerName, this, std::placeholders::_1) });
	//	_commandMap.insert({ "allow_ip", std::bind(&FieldServer::AddConnectPermissionIp, this, std::placeholders::_1) });
	
	}

	void FieldServer::ReceiveExternalCommand(std::string& input)
	{
		// 외부 명령어를 처리하는 로직을 구현합니다.
		// 예를 들어, 서버 상태 확인, 클라이언트 관리 등의 작업을 수행할 수 있습니다.

		std::string command;
		std::vector<std::string> parameters;

		// '['와 ']' 사이 문자열 추출
		size_t start = input.find('[');
		size_t end = input.find(']');

		if (start == std::string::npos || end == std::string::npos || end <= start)
			return; // 잘못된 형식

		command = input.substr(0, start);
		std::string content = input.substr(start + 1, end - start - 1);

		// ',' 기준으로 나누기
		std::stringstream ss(content);
		std::string parameter;
		while (std::getline(ss, parameter, ','))
		{
			//공백 제거
			size_t first = parameter.find_first_not_of(" \t");
			size_t last = parameter.find_last_not_of(" \t");
			if (first != std::string::npos && last != std::string::npos)
				parameter = parameter.substr(first, last - first + 1);

			parameters.push_back(parameter);
		}


		if(_commandMap.find(command) != _commandMap.end())
		{
			// 해당 명령어가 등록되어 있는 경우
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
//			_serverPort = 0; // 초기화
//			DebugLog(Debug::DEBUG_ERROR, std::format("GetServerPort: Invalid port input '{}'. Error: {}", port, e.what()));
//		}
//		
//		//finally
//		::SetEvent(_powerOnEvent);  // Windows API 기반이라면
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
//		::SetEvent(_powerOnEvent);  // Windows API 기반이라면
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