#include "NetworkBaseServer.h"

namespace Network
{
	NetworkBaseServer::NetworkBaseServer()
	{

	}

	NetworkBaseServer::~NetworkBaseServer()
	{

	}

	void NetworkBaseServer::Initialize(int maxClient, int overlappedCount)
	{
		// CPU 갯수
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		_cpuCount = sysInfo.dwNumberOfProcessors;
		if (_cpuCount < 1)
			_cpuCount = 1;

		//if (overlappedCount <= 0)
		//	_overlappedCount = MAX_OVERLAPPED_COUNT; // 기본값

		if (maxClient > CLIENT_MAX)
			maxClient = CLIENT_MAX;

		_maxClient = maxClient;

		_clientManager = new ClientManager();
		_clientManager->InitializeBase(maxClient);

		_overlappedManager = new OverlappedManager();

		DebugLog(Debug::DEBUG_LOG, std::format("NetworkBaseServer Initialized: MaxClient = {}, OverlappedCount = {}", _maxClient, overlappedCount));
	}

	int NetworkBaseServer::StartIOCP()
	{
		int retCode;
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData); // Winsock 초기화

		_serverSocket = ::WSASocket(
			PF_INET,
			SOCK_STREAM,
			IPPROTO_TCP,
			NULL,
			0,
			WSA_FLAG_OVERLAPPED);

		if (_serverSocket == SOCKET_ERROR)
		{
			int errCode = WSAGetLastError();
			DebugLog(Debug::DEBUG_ERROR, std::format("StartListenThread WSASocket failed: {}", errCode));
			WSACleanup();
			_serverOn = false;
			return NETWORK_ERROR;
		}

		// Server 의 ip Address 를 얻는다.
		char szHostName[128] = { 0 };
		gethostname(szHostName, sizeof(szHostName));

		addrinfo hints = {};
		hints.ai_family = AF_INET;  // IPv4
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		addrinfo* result = nullptr;
		if (getaddrinfo(szHostName, nullptr, &hints, &result) == 0)
		{
			for (addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
				sockaddr_in* addr = (sockaddr_in*)ptr->ai_addr;
				char ip[16];
				inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
				//printf("IP: %s\n", ip);
			}
			freeaddrinfo(result);
		}

		BOOL val = TRUE;
		if (setsockopt(_serverSocket, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&val, sizeof(val)) != 0)
		{
			int errCode = WSAGetLastError();
			DebugLog(Debug::DEBUG_ERROR, std::format("setsockopt error: {}", errCode));
			closesocket(_serverSocket);
			WSACleanup();
			_serverOn = false;
			return NETWORK_ERROR;
		}

		struct sockaddr_in serverAddr {};
		serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_port = ::htons(_serverPort);

		retCode = ::bind(_serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
		if (retCode == SOCKET_ERROR)
		{
			int errCode = WSAGetLastError();
			DebugLog(Debug::DEBUG_ERROR, std::format("bind error: {}", errCode));

			::closesocket(_serverSocket);
			::WSACleanup();
			_serverOn = false;
			return NETWORK_ERROR;
		}

		retCode = ::listen(_serverSocket, SOMAXCONN);
		if (retCode == SOCKET_ERROR)
		{
			int errCode = WSAGetLastError();
			DebugLog(Debug::DEBUG_ERROR, std::format("listen error: {}", errCode));

			::closesocket(_serverSocket);
			::WSACleanup();
			_serverOn = false;
			return NETWORK_ERROR;
		}

		_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

		if (!_iocpHandle)
		{
			//sc::writeLogError(std::string("CreateIoCompletionPort error"));
			_serverOn = false;
			return NETWORK_ERROR;
		}
		else
		{
			//sc::writeLogInfo(std::string("CreateIoCompletionPort ok"));
			return NETWORK_OK;
		}

		CreateIoCompletionPort((HANDLE)_serverSocket, _iocpHandle, (ULONG_PTR)this, 0);

		DebugLog(Debug::DEBUG_LOG, std::format("StartIOCP Complete: ServerPort = {}, HostName = {}", _serverPort, _hostName));
	}

	int NetworkBaseServer::StartWorkThreads()
	{
		// Work Thread 갯수 결정
		// CPU 갯수 X 2 가 기본값이다.
		//_cpuCount = sc::getProcessorNumber();
		_workThreadCount = _cpuCount * RATIO_CPU_THREAD; // Processor * 2

		// 설정파일의 최대값 보다 크면 설정파일에 적힌 최대값으로 제한한다.
		//if (_workThreadCount > (DWORD)pConfig->GetMaxThread())
		//	_workThreadCount = (DWORD)pConfig->GetMaxThread();

		// 미리 정의된 최대값보다 크면 프로그램의 정의된 최대값으로 제한한다.
		if (_workThreadCount > WORKER_THREAD_MAX)
			_workThreadCount = WORKER_THREAD_MAX;

		DWORD dwThreadId;

		//sc::writeLogInfo(sc::string::format("%d worker thread", _workThreadCount));

		for (DWORD dwCPU = 0; dwCPU < _workThreadCount; dwCPU++)
		{
			HANDLE hThread;

			hThread = (HANDLE) ::_beginthreadex(
				NULL,
				0,
				WorkThreadProcess,
				this,
				0,
				(unsigned int*)&dwThreadId);

			if (hThread == NULL)
			{
				DebugLog(Debug::DEBUG_ERROR, std::format("{} + Worker thread create error code: +{}", dwCPU + 1, GetLastError()));

				return NETWORK_ERROR;
			}
			else
			{
				// The system schedules threads on their preferred processors whenever possible.
				::SetThreadIdealProcessor(hThread, dwCPU % _cpuCount);
			}

			// store thread handle
			_workerThread[dwCPU] = hThread;
			::CloseHandle(hThread);
		}

		DebugLog(Debug::DEBUG_LOG, "StartWorkThreads Complete");
		return NETWORK_OK;
	}

	int NetworkBaseServer::StopWorkThreads()
	{
		//temp
		DWORD m_dwCompKey = 10;
		//DWORD m_dwCompKey = (DWORD)((m_nMaxClient * 2) + 1);

		for (DWORD dwNum = 0; dwNum < _workThreadCount; dwNum++)
		{
			::PostQueuedCompletionStatus(_iocpHandle,
				0,
				m_dwCompKey,
				NULL);
		}
		// Wait until all worker thread exit
		Sleep(1000);

		DebugLog(Debug::DEBUG_LOG, "StopWorkThreads Complete");
		return NETWORK_OK;
	}

	int NetworkBaseServer::StartListenThread()
	{
		DWORD	dwThreadId;

		_acceptThread = (HANDLE) ::_beginthreadex(
			NULL,
			0,
			AcceptThreadProcess,
			this,
			0,
			(unsigned int*)&dwThreadId);

		if (_acceptThread == NULL)
		{
			DebugLog(Debug::DEBUG_ERROR, "StartListenThread Fail");
			_serverOn = false;
			return NETWORK_ERROR;
		}
		else
		{
			DebugLog(Debug::DEBUG_LOG, "StartListenThread Complete");
			return NETWORK_OK;
		}
	}

	int	NetworkBaseServer::StopListenThread()
	{
		// Close socket, will close Listen Thread
		int nRetCode = ::closesocket(_serverSocket);

		SAFE_CLOSE_HANDLE(_acceptThread);
		_acceptThread = NULL;

		if (nRetCode == SOCKET_ERROR)
		{
			//RAN_TRACE("StopListenThread closesocket ERROR \n");
			return NETWORK_ERROR;
		}
		else
		{
			return NETWORK_OK;
		}
	}

	void NetworkBaseServer::SetUpdateFrame(DWORD dwFrame)
	{
		_updateFrame = dwFrame;
		_updateWaitTime = 1000 / _updateFrame;
	}

	// Start game update thread	
	int NetworkBaseServer::StartUpdateThread()
	{
		SAFE_CLOSE_HANDLE(_updateQuitEvent);

		if (_useEventThread)
			_updateQuitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

		DWORD dwThreadId = 0;

		_updateThread = (HANDLE) ::_beginthreadex(
			NULL,
			0,
			UpdateThreadProcess,
			this,
			CREATE_SUSPENDED,
			(unsigned int*)&dwThreadId);

		if (_updateThread == NULL)
		{
			DebugLog(Debug::DEBUG_ERROR, std::format("Server update thread create failed error code: {}", GetLastError()));
			_serverOn = false;

			return NETWORK_ERROR;
		}
		else
		{
			if (_cpuCount > 2)
			{
				// Update Thread 의 수행레벨을 높여 성능을 향상시킨다.
				// 서버의 CPU 갯수가 2개 이상일때만 실제로 효과가 있었다.
				::SetThreadPriority(_updateThread, THREAD_PRIORITY_HIGHEST);
			}
			//sc::writeLogInfo(std::string("Server update thread create ok"));

			if (_useEventThread)
			{
				SetUpdateFrame(_fps);
				_updateOldTime = ::GetTickCount(); //현재시간 가져오기
			}

			::ResumeThread(_updateThread);

			DebugLog(Debug::DEBUG_LOG, "StartUpdateThread Complete");
			return NETWORK_OK;
		}
	}

	int NetworkBaseServer::SendMessageToClient(DWORD completionKey, std::shared_ptr<MessageData> messageData)
	{
		Network::NetworkUser* user = _clientManager->GetNetworkUser(completionKey);
		if (user != nullptr)
		{
			int resultCode = 0;
			DWORD flags = 0;
			DWORD sendSize = messageData->OverlappedSize();

			SOCKET socket = user->GetSocket();
			Network::CustomOverlapped* overlapped = _overlappedManager->Pop(Network::OperationType::OP_SEND);
			overlapped->CopyFromMessageData(*messageData);

			resultCode = ::WSASend(socket,
				&(overlapped->Wsabuf),
				1,
				&sendSize,
				flags,
				overlapped,
				NULL);

			if (resultCode == SOCKET_ERROR)
			{
				resultCode = ::WSAGetLastError();

				if (resultCode != WSA_IO_PENDING)
				{
					DebugLog(Debug::DEBUG_ERROR, std::format("WSASend error: {}", resultCode));
					return NETWORK_ERROR;
				}

			}

			return NETWORK_OK;
		}
		
		DebugLog(Debug::DEBUG_ERROR, std::format("SendMessageToClient failed: No user found for completionKey: {}", completionKey));
		return NETWORK_ERROR;
	}

	Network::NetworkUser* NetworkBaseServer::GetNetworkUser(DWORD completionKey)
	{
		if (_clientManager == nullptr)
		{
			DebugLog(Debug::DEBUG_ERROR, "ClientManager is not initialized.");
			return nullptr;
		}

		return _clientManager->GetNetworkUser(completionKey);
	}

	int NetworkBaseServer::DisconnectClient(DWORD completionKey)
	{
		auto networkUser = _clientManager->GetNetworkUser(completionKey);
		if(networkUser == nullptr)
		{
			DebugLog(Debug::DEBUG_ERROR, std::format("DisconnectClient failed: No user found for completionKey: {}", completionKey));
			return NETWORK_ERROR;
		}

		SOCKET& socket = networkUser->GetSocket();
		if (socket == INVALID_SOCKET)
		{
			DebugLog(Debug::DEBUG_ERROR, std::format("DisconnectClient failed: Invalid socket for completionKey: {}", completionKey));
			return NETWORK_ERROR;
		}

		LINGER linger;
		linger.l_onoff = TRUE;// 소켓이 닫힐 때 대기 시간을 활성화한다.
		linger.l_linger = 0;// 대기 시간을 0으로 설정하여 즉시 소켓을 닫는다.
		setsockopt(socket, SOL_SOCKET, SO_LINGER, (const char*)&linger, sizeof(linger));
		::shutdown(socket, SD_BOTH);

		int retCode = ::closesocket(socket);
		if (retCode == SOCKET_ERROR)
		{
			retCode = ::WSAGetLastError();
			// A successful WSAStartup call must occur before using this function.
			if (retCode == WSANOTINITIALISED)
			{
				DebugLog(Debug::DEBUG_ERROR, std::format("CloseClient closesocket WSANOTINITIALISED ERROR"));
			}
			// The network subsystem has failed.
			else if (retCode == WSAENETDOWN)
			{
				DebugLog(Debug::DEBUG_ERROR, std::format("CloseClient closesocket WSAENETDOWN ERROR"));
			}
			// The descriptor is not a socket.
			else if (retCode == WSAENOTSOCK)
			{
				DebugLog(Debug::DEBUG_ERROR, std::format("CloseClient closesocket WSAENOTSOCK ERROR"));
			}
			// A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.
			else if (retCode == WSAEINPROGRESS)
			{
				DebugLog(Debug::DEBUG_ERROR, std::format("CloseClient closesocket WSAEINPROGRESS ERROR"));
			}
			// The (blocking) Windows Socket 1.1 call was canceled through WSACancelBlockingCall.
			else if (retCode == WSAEINTR)
			{
				DebugLog(Debug::DEBUG_ERROR, std::format("CloseClient closesocket WSAEINTR ERROR"));
			}
			// The socket is marked as nonblocking and SO_LINGER is set to a nonzero time-out value.
			else if (retCode == WSAEWOULDBLOCK)
			{
				DebugLog(Debug::DEBUG_ERROR, std::format("CloseClient closesocket WSAEWOULDBLOCK ERROR"));
			}
			else
			{
				DebugLog(Debug::DEBUG_ERROR, std::format("CloseClient closesocket unknown ERROR %d", retCode));
			}
		}

		socket = INVALID_SOCKET;

		networkUser->Deinitialize();

		return NETWORK_OK;
	}

	int NetworkBaseServer::RegistMessageDispatcher()
	{
		// 모든 자식클래스에서 사용할 메세지 디스패처를 등록한다.

		return NETWORK_OK;

	}
}


/*

m_dwUpdateFrame = dwFrame;
	m_dwUpdateWaitTime = 1000/m_dwUpdateFrame;
*/