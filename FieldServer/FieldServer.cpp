#include "FieldServer.h"

namespace Field
{
	FieldServer::FieldServer()
	{

	}

	FieldServer::~FieldServer()
	{

	}

	int FieldServer::PowerOnSequence()
	{
		// ���� ��� ���� �ʿ���
		_serverPort = 9091;
		_hostName = "FieldServer";
		_serverOn = true;
		//

		int maxClient = Field::FieldServerDefine::Instance().GetMaxClient();
		int overlappedCount = Field::FieldServerDefine::Instance().GetOverlappedCount();
		Initialize(maxClient, overlappedCount);

		StartIOCP();

		RegistMessageDispatcher();

		StartWorkThreads();
		StartListenThread();
		SetUpdateFrame(60); // FPS ����
		StartUpdateThread();
		_serverOn = true;

		return NETWORK_OK;
	}

	int FieldServer::WorkProcess()
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
				overlapped->Clear();
				_overlappedManager->Push(overlapped);
				// Send �۾� ó��
				//_sendCallback(overlapped, completionKey);
				break;
			}
			}
		}

		return NETWORK_OK;
	}


	int FieldServer::AcceptProcess()
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
			Field::Player* newPlayer = new Field::Player();
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

			intReturnCode = newAcceptUser->ReceiveReady(overlapped);
			if (intReturnCode == NETWORK_ERROR)
			{
				DebugLog(Debug::DEBUG_ERROR, "AcceptProcess: Failed to initiate receive operation.");
				_overlappedManager->Push(overlapped);
				continue;
			}

			DebugLog(Debug::DEBUG_LOG, std::format("AcceptProcess: New client connected: IP = {}, Port = {}, CompletionKey = {}", stringIp, port, completionKey));
		}

		DebugLog(Debug::DEBUG_LOG, "Accept thread End.");
		return NETWORK_OK;
	}


	int FieldServer::UpdateProcess()
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

			if (timeGetTime() - lastHeartbeatTime >= Field::FieldServerDefine::Instance().GetHeartBeatInterval())
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

	void FieldServer::RecvMessageProcess()
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

	void FieldServer::ReadMessage(std::shared_ptr<Network::MessageData> messageData)
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

	int FieldServer::DisconnectClient(DWORD completionKey)
	{
		Network::NetworkBaseServer::DisconnectClient(completionKey);
		
		return 0;
	}

	void FieldServer::PlayerOnlineCheck(unsigned long long currentTime)
	{
		bool offlineCheck = false;
		int size = _playerMap.size();
		for (int i = 0;i < size; ++i)
		{
			auto it = _playerMap.begin();
			std::advance(it, i);
			if (it == _playerMap.end())
				break;

			auto player = it->second;
			if (player == nullptr)
				continue;

			if (!player->IsHeartbeatTarget())
				continue;

			offlineCheck = player->CheckKickoutTarget(currentTime);
			if (offlineCheck == true)
			{
				// Ÿ�Ӿƿ� Ƚ�� �ʰ��� ������ ������ ���´�.
				// ������ �������� �޽����� ���ذ����Ѵ�.
				// �׽�Ʈ�� : ������ ������ �������� �ٽ� ������ �õ��ϵ��� �Ѵ�.(�����̸�����)

				flatbuffers::FlatBufferBuilder builder;
				builder.Finish(protocol::CreateINNER_CLOSE_CLIENT(builder, player->GetCompletionKey()));

				std::shared_ptr<Network::MessageData> messageData = std::make_shared<Network::MessageData>(
					player->GetCompletionKey(),
					Network::MessageHeader(builder.GetSize(), protocol::MESSAGETYPE::MESSAGETYPE_INNER_CLOSE_CLIENT),
					(char*)builder.GetBufferPointer()
				);

				_messageQueue.push(messageData);

				// �÷��̾ �������� ���·� ����Ǿ��� �� ó��
				//DebugLog(Debug::DEBUG_LOG, std::format("Player is offline: {}", player->GetServerName()));
				//SendTelegramMessage(std::format("Server {} is OUT.", player->GetServerName()));
			}
		}
	}

	void FieldServer::ProcessHeartBeat()
	{
		int size = _playerMap.size();
		flatbuffers::FlatBufferBuilder builder;// while�� �ȿ��� ������ builder�� while������� �����ȴ�.
		builder.Finish(protocol::CreateREQUEST_HEARTBEAT(builder));

		for (int i = 0;i < size; ++i)
		{
			auto it = _playerMap.begin();
			std::advance(it, i);
			if (it == _playerMap.end())
				break;
			auto player = it->second;
			if (player == nullptr)
				continue;

			if (!player->IsHeartbeatTarget())
				continue;

			player->SaveRequestHearbeatTime();

			Network::MessageHeader header(builder.GetSize(), protocol::MESSAGETYPE::MESSAGETYPE_REQUEST_HEARTBEAT);
			std::shared_ptr<Network::MessageData> messageData = std::make_shared<Network::MessageData>(
				player->GetCompletionKey(),
				header,
				(char*)builder.GetBufferPointer()
			);

			SendMessageToClient(player->GetCompletionKey(), messageData);
			//DebugLog(Debug::DEBUG_LOG, std::format("SEND [REQUEST HEARTBEAT] To Player {}.", player->GetServerName()));
		}

		DebugLog(Debug::DEBUG_LOG, "ProcessHeartBeat");
	}

	void FieldServer::SetDebugLogCallback(std::function<void(const std::string&, const std::string&)> callback)
	{
		_debugLogCallback = callback;
	}

	void FieldServer::DebugLog(Debug::DebugType debugtype, const std::string& message)
	{
		if (!_debugLogCallback)
			return;

		if (debugtype < 0 || debugtype > Debug::MAX)
		{
			DebugLog(Debug::DEBUG_ERROR, "Debug type is out of range.");
			return;
		}

		std::string typeString = Debug::EnumNamesDebugType()[debugtype]; // Ensure debugtype is valid
		_debugLogCallback(typeString, message);
	}


	void FieldServer::SendTelegramMessage(const std::string& message)
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