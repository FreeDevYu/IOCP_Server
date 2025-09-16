#include "FieldServer.h"

namespace Field
{
	int FieldServer::RegistMessageDispatcher()
	{
		NetworkBaseServer::RegistMessageDispatcher();

		_messageDispatchers[protocol::MESSAGETYPE::MESSAGETYPE_INNER_CLOSE_CLIENT].ProtocolFunction = std::bind(&FieldServer::INNER_CLOSE_CLIENT, this, std::placeholders::_1, std::placeholders::_2);
	
		_messageDispatchers[protocol::MESSAGETYPE::MESSAGETYPE_REQUEST_REGISTER].ProtocolFunction = std::bind(&FieldServer::REQUEST_REGISTER, this, std::placeholders::_1, std::placeholders::_2);
		_messageDispatchers[protocol::MESSAGETYPE::MESSAGETYPE_RESPONSE_HEARTBEAT].ProtocolFunction = std::bind(&FieldServer::RESPONSE_HEARTBEAT, this, std::placeholders::_1, std::placeholders::_2);

		_messageDispatchers[protocol::MESSAGETYPE::MESSAGETYPE_REQUEST_PLAYERMOVE].ProtocolFunction = std::bind(&FieldServer::REQUEST_PLAYERMOVE, this, std::placeholders::_1, std::placeholders::_2);

		return NETWORK_OK;
	}
}