#include "NetworkUser.h"

namespace Network
{
	NetworkUser::NetworkUser()
		: _messageBuilder(NULL)
		, _completionKey(0)
	{
		
	}

	NetworkUser::~NetworkUser()
	{

	}

	void NetworkUser::Initialize(DWORD completionKey, SOCKET socket, std::string ip, int port)
	{
		_messageBuilder = new MessageBuilder();

		if (_socket != INVALID_SOCKET)
		{
			//sc::writeLogError(std::string("CNetUser::SetAcceptedClient if (socket_ != INVALID_SOCKET)"));

			int nRetCode = ::closesocket(_socket);
			if (nRetCode == SOCKET_ERROR)
			{
				nRetCode = ::WSAGetLastError();
				//sc::writeLogError(sc::string::format("CNetUser::SetAcceptedClient closesocket WSAGetLastError: %d", nRetCode));
			}
		}

		_socket = socket;
		_completionKey = completionKey;
		_ip = ip;
		_port = port;
	}

	void NetworkUser::Deinitialize()
	{
		SAFE_DELETE(_messageBuilder);
		_completionKey = 0;
		_socket = NULL;//socket �ݱ�� clientmanager���� ó���Ѵ�.
	}

	SOCKET& NetworkUser::GetSocket()
	{
		return _socket;
	}

	DWORD NetworkUser::GetCompletionKey() const
	{
		return _completionKey;
	}

	int NetworkUser::AddMessage(void* message, DWORD size)
	{
		if(_messageBuilder == NULL)
		{
			//sc::writeLogError(std::string("CNetUser::AddMessage _messageBuilder is NULL"));
			return NETWORK_ERROR;
		}

		_messageBuilder->InsertMessage(message, size);
	}

	std::shared_ptr<Network::MessageData> NetworkUser::GetReceiveMessage()
	{
		if (IsOnline())
		{
			if (_messageBuilder == NULL)
			{
				//sc::writeLogError(std::string("CNetUser::GetReceiveMessage _messageBuilder is NULL"));
				return nullptr;
			}

			// ���⼭ �޼��������͸� �����´�.

			Network::MessageHeader header{ 0,0 };
			char* bodyBuffer = nullptr;

			int feedback = _messageBuilder->MessageCheckAndReturn(header, bodyBuffer);

			if (feedback == NETWORK_ERROR)
			{
				//sc::writeLogError(std::string("CNetUser::GetReceiveMessage MessageCheckAndReturn failed"));
				return nullptr;
			}

	
			return  std::make_shared<Network::MessageData>(_completionKey,header,bodyBuffer);
		}
		
		return nullptr;
	}

	int NetworkUser::GetPendingIOCount() const
	{
		return _ioPendingCount.load(std::memory_order_acquire);
	}

	void NetworkUser::IncreasePendingIOCount()
	{
		_ioPendingCount.fetch_add(1, std::memory_order_relaxed);

	}

	void NetworkUser::DecreasePendingIOCount()
	{
		if (_ioPendingCount.load(std::memory_order_relaxed) > 0)
		{
			_ioPendingCount.fetch_sub(1, std::memory_order_relaxed);
		}
		else
		{

		}
	}

	void NetworkUser::SetLastRequeueTime(DWORD time)
	{
		_ioLastRequeueTime.store(time, std::memory_order_release);
	}

	DWORD NetworkUser::GetLastRequeueTime() const
	{
		return _ioLastRequeueTime.load(std::memory_order_acquire);
	}

	std::string NetworkUser::GetIpAddress() const 
	{ 
		return _ip;
	}
}