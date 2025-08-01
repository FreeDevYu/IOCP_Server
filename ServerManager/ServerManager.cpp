#include "ServerManager.h"


ServerManager::ServerManager()
{

}

ServerManager::~ServerManager()
{
	int a=5;
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
				int feedback = _clientManager->AddMessageToClient(completionKey, overlapped->Wsabuf.buf, bytesTransferred);//���⼭ �޼������ۿ� ���� �Ͼ.

				void* message = _clientManager->GetReceiveMessageFromClient(completionKey);//���⼭ ���� ���� �Ͼ. �޼������۴� memmove

				while (message != nullptr)
				{
					// �޽��� ó�� ������ �����մϴ�.

					_messageQueue.push(message);
					message = _clientManager->GetReceiveMessageFromClient(completionKey);
				}

				_overlappedManager->Push(overlapped);
				overlapped = _overlappedManager->Pop(Network::OperationType::OP_RECV);

				bytesTransferred = 111; //sizeof(protocol::NetMessageGeneric);

				int nRetCode = ::WSARecv(
					_clientManager->GetSocket(completionKey),
					&(overlapped->Wsabuf),
					1,
					&bytesTransferred,
					&flags,
					&(*overlapped),
					NULL);

				break;
			case Network::OperationType::OP_SEND:
				// Send �۾� ó��
				//_sendCallback(overlapped, completionKey);
				break;


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

	while (_serverOn)
	{
		newAcceptSocket = WSAAccept(_serverSocket, NULL, NULL, NULL, 0);

		if (newAcceptSocket == INVALID_SOCKET)
		{
			intReturnCode = WSAGetLastError();
			///sc::writeLogError(sc::string::format("WSAAccept: %1%", nRetCode));
			if (intReturnCode == WSAENOTSOCK || intReturnCode == WSAEINTR)
				break;
			else
				continue;
		}

		getpeername(newAcceptSocket, (sockaddr*)&addressInfo, &addressInfoSize);
		inet_ntop(AF_INET, &(addressInfo.sin_addr), ip, INET_ADDRSTRLEN);
		port = ntohs(addressInfo.sin_port);

		//�������� üũ�� ���� ���ʿ䰡 ������?

		completionKey = _clientManager->PopCompletionKey();
		if(completionKey == NETWORK_ERROR)
		{
			// �ִ� Ŭ���̾�Ʈ ���� �ʰ��� ��� ������ �ݰ� ���� ������ ��ٸ��ϴ�.
			closesocket(newAcceptSocket);
			continue;
		}

		Network::NetworkUser* newAcceptUser = new Network::NetworkUser();

		feedback =  _clientManager->AddClient(completionKey, newAcceptUser);
		if(feedback == NETWORK_ERROR)
		{
			// Ŭ���̾�Ʈ �߰� ���� �� ������ �ݰ� ���� ������ ��ٸ��ϴ�.
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
			//sc::writeLogError(std::string("CreateIoCompletionPort error"));
			//CloseClient(dwClient);
			continue;
		}

		auto overlapped = _overlappedManager->Pop(Network::OperationType::OP_RECV);
		if(overlapped == nullptr)
		{
			//_clientManager->RemoveClient(completionKey);
			continue;
		}

		bytesTransferred = 111; //sizeof(protocol::NetMessageGeneric);

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
			newAcceptUser->DecreasePendingIOCount();
			//_clientManager->RemoveClient(completionKey); ->job���·� �ٲٱ�.
			_overlappedManager->Push(overlapped);
			continue;
		}
	}

	return NETWORK_OK;
}

int ServerManager::UpdateProcess()
{
	// ���� ������Ʈ ������ �����մϴ�.
	// ���� ���, ���� ���� ������Ʈ, Ŭ���̾�Ʈ���� ������ ���� ���� ������ �� �ֽ��ϴ�.

	DWORD currentTime = 0;
	DWORD offset = 0;
	DWORD quitEventResult = 0;

	while (_serverOn)
	{
		currentTime = timeGetTime();
		offset = currentTime - _updateOldTime;// ���� �ð��� ���� ������Ʈ �ð��� ���̸� ����մϴ�.
		_updateOldTime = currentTime;

		int remainingTime = _updateWaitTime - offset;// ����	 ������Ʈ ��� �ð����� ���� ��� �ð��� ���ϴ�.���߳����� ó���ð����� �����Ӹ��� ���� �ֱ⸦ ��Ű�� ����.
		if(remainingTime < 0)
			remainingTime = 0;

		quitEventResult = WaitForSingleObject(_updateQuitEvent, remainingTime);

		if (quitEventResult == WAIT_OBJECT_0)
		{
			// ������Ʈ �����尡 ���� ��û�� �޾��� �� ó���մϴ�.
			break;
		}
		else if (quitEventResult == WAIT_TIMEOUT)
		{
			// ������Ʈ �۾��� �����մϴ�.

			RecvMessageProcess();
		}
	}

	return NETWORK_OK;
}

void ServerManager::RecvMessageProcess()
{
	void* message = nullptr;

	do
	{
		_messageQueue.try_pop(message);
		ReadMessage(message);

	} while (message != nullptr);
}

void ServerManager::ReadMessage(void* message)
{
	if (message == nullptr)
		return;

	Network::MessageData* messageData = (Network::MessageData*)message;

	if (messageData == nullptr)
		return;

	if (messageData->BodySize == 0)
		return;
	if (messageData->CompletionKey < 0)
		return;

	if (messageData->Header.ContentsType < 0 || messageData->Header.ContentsType > protocol::MESSAGETYPE_MAX)
	{
		return;
	}

	protocol::MESSAGETYPE messageType = (protocol::MESSAGETYPE)messageData->Header.ContentsType;

	_messageDispatchers[messageType].ProtocolFunction(*this, messageData->CompletionKey, messageData->Body);

	//Network::MessageData* messageData = reinterpret_cast<Network::MessageData*>(message);
	//
	//if (messageData != nullptr)
	//{
	//	messageData->Header.
	//}
}
