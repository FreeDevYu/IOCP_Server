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
}