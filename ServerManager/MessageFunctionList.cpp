#include "ServerManager.h"

namespace Manager
{
	int ServerManager::RegistMessageDispatcher()
	{
		NetworkBaseServer::RegistMessageDispatcher();

		_messageDispatchers[protocol::MESSAGETYPE::MESSAGETYPE_INNER_CLOSE_CLIENT].ProtocolFunction = std::bind(&ServerManager::INNER_CLOSE_CLIENT, this, std::placeholders::_1, std::placeholders::_2);

		_messageDispatchers[protocol::MESSAGETYPE::MESSAGETYPE_REQUEST_REGISTER].ProtocolFunction = std::bind(&ServerManager::REQUEST_REGISTER, this, std::placeholders::_1, std::placeholders::_2);
		_messageDispatchers[protocol::MESSAGETYPE::MESSAGETYPE_RESPONSE_HEARTBEAT].ProtocolFunction = std::bind(&ServerManager::RESPONSE_HEARTBEAT, this, std::placeholders::_1, std::placeholders::_2);


		return NETWORK_OK;
	}
}