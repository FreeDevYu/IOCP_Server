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

			//�������� üũ�� ���� ���ʿ䰡 ������?

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

			std::string stringIp(ip);
			newAcceptUser->Initialize(completionKey, newAcceptSocket, stringIp, port);

			handleReturnCode = CreateIoCompletionPort(
				(HANDLE)newAcceptSocket,
				_iocpHandle,
				completionKey,
				0);

			if (!handleReturnCode)
			{
				_clientManager->RemoveClient(completionKey);
				DebugLog(Debug::DEBUG_WARNING, std::format("CreateIoCompletionPort failed for socket {}: {}", newAcceptSocket, GetLastError()));
				//sc::writeLogError(std::string("CreateIoCompletionPort error"));
				//CloseClient(dwClient);
				continue;
			}

			auto overlapped = _overlappedManager->Pop(Network::OperationType::OP_RECV);
			if (overlapped == nullptr)
			{
				DebugLog(Debug::DEBUG_ERROR, "AcceptProcess: Failed to pop overlapped structure.");
				//_clientManager->RemoveClient(completionKey);
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
				continue;
			}

			DebugLog(Debug::DEBUG_LOG, std::format("AcceptProcess: New client connected: IP = {}, Port = {}, CompletionKey = {}", stringIp, port, completionKey));
		}

		DebugLog(Debug::DEBUG_LOG, "Accept thread End.");
		return NETWORK_OK;
	}

	int ServerManager::UpdateProcess()
	{
		// ���� ������Ʈ ������ �����մϴ�.
		// ���� ���, ���� ���� ������Ʈ, Ŭ���̾�Ʈ���� ������ ���� ���� ������ �� �ֽ��ϴ�.

		
		DWORD currentTime = 0;
		DWORD offset = 0;
		DWORD quitEventResult = 0;

		DWORD lastHeartbeatTime = 0;

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
			else if (quitEventResult == WAIT_TIMEOUT)
			{
				// ������Ʈ �۾��� �����մϴ�.

				RecvMessageProcess();

				if(timeGetTime() - lastHeartbeatTime >= Manager::ServerManagerDefine::Instance().GetHeartBeatInterval())
				{
					ProcessHeartBeat();
					lastHeartbeatTime = timeGetTime();
				}
				else
				{
					PlayerOnlineCheck(currentTime);
				}
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
}