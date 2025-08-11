#pragma comment(lib, "winhttp.lib")

#include "ServerManager.h"
#include "ServerManagerDefine.h"


namespace Manager
{

	ServerManager::ServerManager()
	{

	}

	ServerManager::~ServerManager()
	{
		int a = 5;
	}

	int ServerManager::PowerOnSequence()
	{
		// ���� ���� �� �ʱ�ȭ �۾��� �����մϴ�.
		// ���� ���, IOCP �ڵ� ����, ���� ���� ���� ������ �� �ֽ��ϴ�.

		SettingExternalCommands();
		DebugLog(Debug::DEBUG_LOG, "ServerManager PowerOnSequence called.");
		_powerOnEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

		while (true)
		{
			DebugLog(Debug::DEBUG_LOG, "Please Input Server Port... (ex-> Port[9090]");
			WaitForSingleObject(_powerOnEvent, INFINITE);
			ResetEvent(_powerOnEvent); // �̺�Ʈ ����

			if (_serverPort == 0)
				continue;// ��Ʈ�� �������� ���� ��� ��� �Է��� �޽��ϴ�.

			Sleep(1000); // 1�� ���	

			DebugLog(Debug::DEBUG_LOG, "Please Input Server Name... (ex-> name[SeverManager]");
			WaitForSingleObject(_powerOnEvent, INFINITE);
			ResetEvent(_powerOnEvent); // �̺�Ʈ ����

			if (_hostName == "")
				continue; // ȣ��Ʈ �̸��� �������� ���� ��� ��� �Է��� �޽��ϴ�.

			DebugLog(Debug::DEBUG_LOG, std::format("ServerManager PowerOnSequence: ServerPort = {}, HostName = {}", _serverPort, _hostName));
			break;
		}

		int maxClient = Manager::ServerManagerDefine::Instance().GetMaxClient();
		int overlappedCount = Manager::ServerManagerDefine::Instance().GetOverlappedCount();
		Initialize(maxClient, overlappedCount);

		StartIOCP();

		_serverIpSet.insert("127.0.0.1");
		//AddConnectPermissionIp("127.0.0.1");
		RegistMessageDispatcher();

		StartWorkThreads();
		StartListenThread();
		SetUpdateFrame(60); // FPS ����
		StartUpdateThread();
		_serverOn = true;

		return NETWORK_OK;
	}

	int ServerManager::WorkProcess()
	{
		// �۾� �����忡�� ó���� �۾��� �����մϴ�.
		// ���� ���, Ŭ���̾�Ʈ ��û ó��, �����ͺ��̽� ������Ʈ ���� ������ �� �ֽ��ϴ�.


		Network::CustomOverlapped* overlapped = nullptr;
		DWORD bytesTransferred = 0;
		ULONG_PTR completionKey = 0;
		DWORD flags = 0;

		while (_serverOn)
		{
			BOOL result = GetQueuedCompletionStatus(_iocpHandle, &bytesTransferred, &completionKey, reinterpret_cast<LPOVERLAPPED*>(&overlapped), INFINITE);

			if (overlapped != nullptr && overlapped->OperationType == Network::OperationType::OP_END)
			{
				//��������ȣ��
			}

			if ((completionKey < 0) || (completionKey >= _maxClient))
			{
				// �߸��� completionKey�� ���	
				continue;
			}

			if (!result) // ��������
			{
				_clientManager->RemoveClient(completionKey);
				continue;
			}

			if (overlapped->OperationType == Network::OperationType::OP_RECV && bytesTransferred == 0) // ��������
			{
				//CloseClient(dwClient);
				//m_pClientManager->ReleaseOperationData(pPerIoData);
				_clientManager->RemoveClient(completionKey);
				continue;
			}

			switch (overlapped->OperationType)
			{
			case Network::OperationType::OP_RECV:
			{
				int feedback = _clientManager->AddMessageToClient(completionKey, overlapped->Wsabuf.buf, bytesTransferred);//���⼭ �޼������ۿ� ���� �Ͼ.

				std::shared_ptr<Network::MessageData> message = _clientManager->GetReceiveMessageFromClient(completionKey);
				overlapped->Clear();
				_overlappedManager->Push(overlapped);

				while (message != nullptr)
				{
					// �޽��� ó�� ������ �����մϴ�.

					_messageQueue.push(message);
					message = _clientManager->GetReceiveMessageFromClient(completionKey);
				}

				overlapped = _overlappedManager->Pop(Network::OperationType::OP_RECV);
				Network::NetworkUser* user = _clientManager->GetNetworkUser(completionKey);

				int nRetCode = ::WSARecv(
					user->GetSocket(),
					&(overlapped->Wsabuf),
					1,
					&bytesTransferred,
					&flags,
					&(*overlapped),
					NULL);

				break;
			}
				
			case Network::OperationType::OP_SEND:
			{
				// Send �۾� ó��
				//_sendCallback(overlapped, completionKey);
				break;
			}
			}
		}

		return NETWORK_OK;
	}

	int ServerManager::AcceptProcess()
	{
		// Ŭ���̾�Ʈ ������ �����ϴ� ������ �����մϴ�.
		// ���� ���, ���ο� Ŭ���̾�Ʈ ������ �����ϰ�, IOCP�� ����ϴ� ���� �۾��� ������ �� �ֽ��ϴ�.

		SOCKET newAcceptSocket;
		int intReturnCode = 0;
		HANDLE handleReturnCode = NULL;

		sockaddr_in	addressInfo;
		int addressInfoSize = sizeof(addressInfo);
		char ip[MAX_IP_LENGTH + 1] = { 0 };
		int port = 0;
		DWORD completionKey = NETWORK_ERROR;
		int feedback = NETWORK_ERROR;

		DWORD bytesTransferred = 0;
		DWORD flags = 0;

		DebugLog(Debug::DEBUG_LOG, "Accept thread Start...");
		while (_serverOn)
		{
			newAcceptSocket = WSAAccept(_serverSocket, NULL, NULL, NULL, 0);

			if (newAcceptSocket == INVALID_SOCKET)
			{
				intReturnCode = WSAGetLastError();

				///sc::writeLogError(sc::string::format("WSAAccept: %1%", nRetCode));
				if (intReturnCode == WSAENOTSOCK || intReturnCode == WSAEINTR)
				{
					DebugLog(Debug::DEBUG_ERROR, std::format("WSAAccept failed with error code: {}", intReturnCode));
					break;
				}
				else
					continue;
			}

			getpeername(newAcceptSocket, (sockaddr*)&addressInfo, &addressInfoSize);
			inet_ntop(AF_INET, &(addressInfo.sin_addr), ip, INET_ADDRSTRLEN);
			port = ntohs(addressInfo.sin_port);
			std::string stringIp(ip);	
			
			completionKey = _clientManager->PopCompletionKey();
			if (completionKey == NETWORK_ERROR)
			{
				DebugLog(Debug::DEBUG_WARNING, "AcceptProcess: No available completion key.");
				// �ִ� Ŭ���̾�Ʈ ���� �ʰ��� ��� ������ �ݰ� ���� ������ ��ٸ��ϴ�.
				closesocket(newAcceptSocket);
				continue;
			}

			Network::NetworkUser* newAcceptUser = new Network::NetworkUser();

			feedback = _clientManager->AddClient(completionKey, newAcceptUser);
			if (feedback == NETWORK_ERROR)
			{
				// Ŭ���̾�Ʈ �߰� ���� �� ������ �ݰ� ���� ������ ��ٸ��ϴ�.
				DebugLog(Debug::DEBUG_ERROR, "AcceptProcess: Failed to add new client.");
				closesocket(newAcceptSocket);
				continue;
			}

			newAcceptUser->Initialize(completionKey, newAcceptSocket, stringIp, port);

			// ���ο� �÷��̾� �߰�
			Manager::Player* newPlayer = new Manager::Player();
			newPlayer->Initialize(completionKey, timeGetTime(), _debugLogCallback);
			_playerMap.insert({ completionKey, newPlayer });

			handleReturnCode = CreateIoCompletionPort(
				(HANDLE)newAcceptSocket,
				_iocpHandle,
				completionKey,
				0);

			if (!handleReturnCode)
			{
				_clientManager->RemoveClient(completionKey);
				DebugLog(Debug::DEBUG_WARNING, std::format("CreateIoCompletionPort failed for socket {}: {}", newAcceptSocket, GetLastError()));

				continue;
			}

			auto overlapped = _overlappedManager->Pop(Network::OperationType::OP_RECV);
			if (overlapped == nullptr)
			{
				DebugLog(Debug::DEBUG_ERROR, "AcceptProcess: Failed to pop overlapped structure.");
				continue;
			}

			int nRetCode = ::WSARecv(
				newAcceptSocket,
				&(overlapped->Wsabuf),
				1,
				&bytesTransferred,
				&flags,
				&(*overlapped),
				NULL);

			newAcceptUser->IncreasePendingIOCount();
			newAcceptUser->SetLastRequeueTime(GetTickCount());

			int nLastErr = 0;
			if ((nRetCode == SOCKET_ERROR) && ((nLastErr = WSAGetLastError()) != WSA_IO_PENDING))
			{
				DebugLog(Debug::DEBUG_WARNING, std::format("WSARecv failed for socket {}: {}", newAcceptSocket, nLastErr));
				newAcceptUser->DecreasePendingIOCount();
				//_clientManager->RemoveClient(completionKey); ->job���·� �ٲٱ�.
				_overlappedManager->Push(overlapped);
			}
			else
			{
				DebugLog(Debug::DEBUG_LOG, std::format("WSARecv initiated for socket {}: bytesTransferred = {}, flags = {}", newAcceptSocket, bytesTransferred, flags));
			}

			DebugLog(Debug::DEBUG_LOG, std::format("AcceptProcess: New client connected: IP = {}, Port = {}, CompletionKey = {}", stringIp, port, completionKey));
		}

		DebugLog(Debug::DEBUG_LOG, "Accept thread End.");
		return NETWORK_OK;
	}

	int ServerManager::UpdateProcess()
	{
		// ���� ������Ʈ ������ �����մϴ�.
		DWORD currentTime = 0;
		DWORD offset = 0;
		DWORD quitEventResult = 0;

		unsigned long long lastHeartbeatTime = 0;

		while (_serverOn)
		{
			currentTime = timeGetTime();
			offset = currentTime - _updateOldTime;// ���� �ð��� ���� ������Ʈ �ð��� ���̸� ����մϴ�.
			_updateOldTime = currentTime;

			int remainingTime = _updateWaitTime - offset;// ����	 ������Ʈ ��� �ð����� ���� ��� �ð��� ���ϴ�.���߳����� ó���ð����� �����Ӹ��� ���� �ֱ⸦ ��Ű�� ����.
			if (remainingTime < 0)
				remainingTime = 0;
			
			quitEventResult = WaitForSingleObject(_updateQuitEvent, remainingTime);

			if (quitEventResult == WAIT_OBJECT_0)
			{
				DebugLog(Debug::DEBUG_LOG, "Update thread quit event received.");
				// ������Ʈ �����尡 ���� ��û�� �޾��� �� ó���մϴ�.
				break;
			}
			
			RecvMessageProcess();

			if (timeGetTime() - lastHeartbeatTime >= Manager::ServerManagerDefine::Instance().GetHeartBeatInterval())
			{
				ProcessHeartBeat();
				lastHeartbeatTime = timeGetTime();
			}
			else
			{
				PlayerOnlineCheck(GetTickCount64());
			}
		}

		return NETWORK_OK;
	}

	void ServerManager::RecvMessageProcess()
	{
		std::shared_ptr<Network::MessageData> messageData = nullptr;
		bool feedback = false;

		do
		{
			feedback = _messageQueue.try_pop(messageData);
			if (feedback)
			{
				ReadMessage(messageData);
			}
			

		} while (feedback);
	}

	void ServerManager::ReadMessage(std::shared_ptr<Network::MessageData> messageData)
	{
		if (messageData == nullptr || messageData->BodySize < 1 || messageData->CompletionKey < 0)
		{
			DebugLog(Debug::DEBUG_ERROR, "ReadMessage: Invalid message data.");
			return;
		}

		if (messageData->Header.ContentsType < protocol::MESSAGETYPE_MIN || messageData->Header.ContentsType > protocol::MESSAGETYPE_MAX)
		{
			DebugLog(Debug::DEBUG_ERROR, "ReadMessage: Invalid message type.");
			return;
		}

		protocol::MESSAGETYPE messageType = (protocol::MESSAGETYPE)messageData->Header.ContentsType;

		_messageDispatchers[messageType].ProtocolFunction(*this, messageData);
	}

	void ServerManager::SetDebugLogCallback(std::function<void(const std::string&, const std::string&)> callback)
	{
		_debugLogCallback = callback;
	}

	void ServerManager::DebugLog(Debug::DebugType debugtype, const std::string& message)
	{
		if (!_debugLogCallback)
			return;

		if(debugtype < 0 || debugtype > Debug::MAX)
		{
			DebugLog(Debug::DEBUG_ERROR, "Debug type is out of range.");
			return;
		}

		std::string typeString = Debug::EnumNamesDebugType()[debugtype]; // Ensure debugtype is valid
		_debugLogCallback(typeString, message);
	}


	void ServerManager::SendTelegramMessage(const std::string& message)
	{
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, message.c_str(), (int)message.size(), NULL, 0);
		std::wstring wstr(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, message.c_str(), (int)message.size(), &wstr[0], size_needed);




		std::wstring token = L"8470620144:AAHYPfRumJvLjo7tEBR0OknoLF0Wz-ed3io";
		std::wstring chatId = L"8349626032";

		std::wstring host = L"api.telegram.org";
		std::wstring path = L"/bot" + token + L"/sendMessage?chat_id=" + chatId + L"&text=" + wstr;

		HINTERNET hSession = WinHttpOpen(L"TelegramBot/1.0",
			WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS, 0);

		HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(),
			INTERNET_DEFAULT_HTTPS_PORT, 0);

		HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),
			NULL, WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			WINHTTP_FLAG_SECURE);

		BOOL bResult = WinHttpSendRequest(hRequest,
			WINHTTP_NO_ADDITIONAL_HEADERS, 0,
			WINHTTP_NO_REQUEST_DATA, 0,
			0, 0);

		if (bResult)
			WinHttpReceiveResponse(hRequest, NULL);

		// ������ ���� Ȯ��
		DWORD dwSize = 0;
		WinHttpQueryDataAvailable(hRequest, &dwSize);
		if (dwSize > 0)
		{
			std::wstring response(dwSize, L'\0');
			DWORD dwDownloaded = 0;
			WinHttpReadData(hRequest, &response[0], dwSize, &dwDownloaded);
			wprintf(L"Telegram Response: %s\n", response.c_str());
		}

		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
	}


}