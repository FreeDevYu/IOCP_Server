#include "NetworkBaseServer.h"

namespace Network
{
	NetworkBaseServer::NetworkBaseServer()
	{

	}

	NetworkBaseServer::~NetworkBaseServer()
	{

	}

	void NetworkBaseServer::Initialize(Network::BaseClientManager* clientManager, Network::OverlappedManager* overlappedManager, int serverPort, std::string hostName, int overlappedCount, int maxClient)
	{
		_serverOn = true;
		_serverPort = serverPort;
		_hostName = hostName;

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

		if (clientManager == nullptr)
			clientManager = new BaseClientManager();

		_clientManager = clientManager;
		_clientManager->InitializeBase(maxClient);

		_overlappedManager = overlappedManager;;
	}

	int NetworkBaseServer::StartIOCP()
	{
		if (!_iocpHandle)
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
				//sc::writeLogError(sc::string::format("%d Worker thread create error code: %d", dwCPU + 1, GetLastError()));
				return NETWORK_ERROR;
			}
			else
			{
				//sc::writeLogInfo(sc::string::format("#%1% Worker thread create ok", dwCPU + 1));
				// The system schedules threads on their preferred processors whenever possible.
				::SetThreadIdealProcessor(hThread, dwCPU % _cpuCount);
			}
			// store thread handle
			_workerThread[dwCPU] = hThread;
			::CloseHandle(hThread);
		}
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
		return NETWORK_OK;
	}

	int NetworkBaseServer::StartListenThread()
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
			//sc::writeLogError(sc::string::format("StartListenThread WSASocket failed: %1%", sc::net::getLastError(errCode)));
			WSACleanup();
			_serverOn = false;
			return NETWORK_ERROR;
		}
		else
		{
			//sc::writeLogInfo(std::string("WSASocket ok"));
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
			//sc::writeLogError(sc::string::format("setsockopt error: %1%", sc::net::getLastError(errCode)));
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
			//sc::writeLogError(
			//	sc::string::format(
			//		"bind error: %1%",
			//		sc::net::getLastError(errCode)));
			::closesocket(_serverSocket);
			::WSACleanup();
			_serverOn = false;
			return NETWORK_ERROR;
		}
		else
		{
			//sc::writeLogInfo(sc::string::format("Bind port: %1%", _serverPort));
		}

		retCode = ::listen(_serverSocket, SOMAXCONN);
		if (retCode == SOCKET_ERROR)
		{
			int errCode = WSAGetLastError();
			//sc::writeLogError(sc::string::format("listen error: %1%", sc::net::getLastError(errCode)));
			::closesocket(_serverSocket);
			::WSACleanup();
			_serverOn = false;
			return NETWORK_ERROR;
		}

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
			//sc::writeLogError(sc::string::format("Server accept thread create failed: %d", GetLastError()));
			_serverOn = false;
			return NETWORK_ERROR;
		}
		else
		{
			//sc::writeLogInfo(std::string("Server accept thread create ok"));
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
			//sc::writeLogError(sc::string::format("Server update thread create failed error code: %d", GetLastError()));
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

			return NETWORK_OK;
		}
	}
}

/*

m_dwUpdateFrame = dwFrame;
	m_dwUpdateWaitTime = 1000/m_dwUpdateFrame;
*/