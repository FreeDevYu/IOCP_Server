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

		//if (_socket != INVALID_SOCKET)
		//{
		//	int ret = closesocket(_socket);
		//	if (ret == SOCKET_ERROR)
		//	{
		//		int errCode = WSAGetLastError();
		//		// Handle error if needed
		//	}
		//}

		_socket = INVALID_SOCKET;
	}

	SOCKET NetworkUser::GetSocket()
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

	void* NetworkUser::GetReceiveMessage()
	{
		if (IsOnline())
		{
			if (_messageBuilder == NULL)
			{
				//sc::writeLogError(std::string("CNetUser::GetReceiveMessage _messageBuilder is NULL"));
				return nullptr;
			}

			char* outBuffer = nullptr;
			int bufferSize = 0;

			int feedback = _messageBuilder->MessageCheckAndReturn(outBuffer, bufferSize);

			if (feedback == NETWORK_ERROR || bufferSize < sizeof(MessageHeader))
			{
				//sc::writeLogError(std::string("CNetUser::GetReceiveMessage MessageCheckAndReturn failed"));
				return nullptr;
			}

			return outBuffer;
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