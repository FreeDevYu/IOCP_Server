#include "ServerManager.h"

namespace Manager
{
	int ServerManager::RegistMessageDispatcher()
	{
		NetworkBaseServer::RegistMessageDispatcher();

		_messageDispatchers[protocol::MESSAGETYPE::MESSAGETYPE_REQUEST_CONNECT].ProtocolFunction = std::bind(&ServerManager::REQUEST_CONNECT, this, std::placeholders::_1, std::placeholders::_2);
		_messageDispatchers[protocol::MESSAGETYPE::MESSAGETYPE_RESPONSE_CONNECT].ProtocolFunction = std::bind(&ServerManager::RESPONSE_CONNECT, this, std::placeholders::_1, std::placeholders::_2);

		_messageDispatchers[protocol::MESSAGETYPE::MESSAGETYPE_REQUEST_DISCONNECT].ProtocolFunction = std::bind(&ServerManager::REQUEST_DISCONNECT, this, std::placeholders::_1, std::placeholders::_2);
		_messageDispatchers[protocol::MESSAGETYPE::MESSAGETYPE_RESPONSE_DISCONNECT].ProtocolFunction = std::bind(&ServerManager::RESPONSE_DISCONNECT, this, std::placeholders::_1, std::placeholders::_2);

		_messageDispatchers[protocol::MESSAGETYPE::MESSAGETYPE_NOTICE_KICK].ProtocolFunction = std::bind(&ServerManager::NOTICE_KICK, this, std::placeholders::_1, std::placeholders::_2);

		_messageDispatchers[protocol::MESSAGETYPE::MESSAGETYPE_REQUEST_HEARTBEAT].ProtocolFunction = std::bind(&ServerManager::REQUEST_HEARTBEAT, this, std::placeholders::_1, std::placeholders::_2);
		_messageDispatchers[protocol::MESSAGETYPE::MESSAGETYPE_RESPONSE_HEARTBEAT].ProtocolFunction = std::bind(&ServerManager::RESPONSE_HEARTBEAT, this, std::placeholders::_1, std::placeholders::_2);




		return NETWORK_OK;

	}
}