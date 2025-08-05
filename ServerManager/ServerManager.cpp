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
		// 작업 스레드에서 처리할 작업을 구현합니다.
		// 예를 들어, 클라이언트 요청 처리, 데이터베이스 업데이트 등을 수행할 수 있습니다.


		Network::CustomOverlapped* overlapped = nullptr;
		DWORD bytesTransferred = 0;
		ULONG_PTR completionKey = 0;
		DWORD flags = 0;

		while (_serverOn)
		{
			BOOL result = GetQueuedCompletionStatus(_iocpHandle, &bytesTransferred, &completionKey, reinterpret_cast<LPOVERLAPPED*>(&overlapped), INFINITE);

			if (overlapped != nullptr && overlapped->OperationType == Network::OperationType::OP_END)
			{
				//서버종료호출
			}

			if ((completionKey < 0) || (completionKey >= _maxClient))
			{
				// 잘못된 completionKey인 경우	
				continue;
			}

			if (!result) // 강제종료
			{
				_clientManager->RemoveClient(completionKey);
				continue;
			}

			if (overlapped->OperationType == Network::OperationType::OP_RECV && bytesTransferred == 0) // 정상종료
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
				int feedback = _clientManager->AddMessageToClient(completionKey, overlapped->Wsabuf.buf, bytesTransferred);//여기서 메세지버퍼에 복사 일어남.

				std::shared_ptr<Network::MessageData> message = _clientManager->GetReceiveMessageFromClient(completionKey);
				overlapped->Clear();
				_overlappedManager->Push(overlapped);

				while (message != nullptr)
				{
					// 메시지 처리 로직을 구현합니다.

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
				// Send 작업 처리
				//_sendCallback(overlapped, completionKey);
				break;
			}
			}
		}

		return NETWORK_OK;
	}

	int ServerManager::AcceptProcess()
	{
		// 클라이언트 연결을 수락하는 로직을 구현합니다.
		// 예를 들어, 새로운 클라이언트 소켓을 생성하고, IOCP에 등록하는 등의 작업을 수행할 수 있습니다.

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
					break;
				else
					continue;
			}

			getpeername(newAcceptSocket, (sockaddr*)&addressInfo, &addressInfoSize);
			inet_ntop(AF_INET, &(addressInfo.sin_addr), ip, INET_ADDRSTRLEN);
			port = ntohs(addressInfo.sin_port);

			//서버인지 체크를 먼저 할필요가 있을까?

			completionKey = _clientManager->PopCompletionKey();
			if (completionKey == NETWORK_ERROR)
			{
				DebugLog(Debug::DEBUG_WARNING, "AcceptProcess: No available completion key.");
				// 최대 클라이언트 수를 초과한 경우 소켓을 닫고 다음 연결을 기다립니다.
				closesocket(newAcceptSocket);
				continue;
			}

			Network::NetworkUser* newAcceptUser = new Network::NetworkUser();

			feedback = _clientManager->AddClient(completionKey, newAcceptUser);
			if (feedback == NETWORK_ERROR)
			{
				// 클라이언트 추가 실패 시 소켓을 닫고 다음 연결을 기다립니다.
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
				//_clientManager->RemoveClient(completionKey); ->job형태로 바꾸기.
				_overlappedManager->Push(overlapped);
				continue;
			}
		}

		DebugLog(Debug::DEBUG_LOG, "Accept thread End.");
		return NETWORK_OK;
	}

	int ServerManager::UpdateProcess()
	{
		// 게임 업데이트 로직을 구현합니다.
		// 예를 들어, 게임 상태 업데이트, 클라이언트에게 데이터 전송 등을 수행할 수 있습니다.

		
		DWORD currentTime = 0;
		DWORD offset = 0;
		DWORD quitEventResult = 0;

		DWORD lastHeartbeatTime = 0;

		while (_serverOn)
		{
			currentTime = timeGetTime();
			offset = currentTime - _updateOldTime;// 현재 시간과 이전 업데이트 시간의 차이를 계산합니다.
			_updateOldTime = currentTime;

			int remainingTime = _updateWaitTime - offset;// 원래	 업데이트 대기 시간에서 현재 경과 시간을 뺍니다.들쭉날쭉한 처리시간에서 프레임마다 루프 주기를 지키기 위함.
			if (remainingTime < 0)
				remainingTime = 0;

			quitEventResult = WaitForSingleObject(_updateQuitEvent, remainingTime);

			if (quitEventResult == WAIT_OBJECT_0)
			{
				DebugLog(Debug::DEBUG_LOG, "Update thread quit event received.");
				// 업데이트 스레드가 종료 요청을 받았을 때 처리합니다.
				break;
			}
			else if (quitEventResult == WAIT_TIMEOUT)
			{
				// 업데이트 작업을 수행합니다.

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

	void ServerManager::ReceiveExternalCommand(const std::string& command)
	{
		// 외부 명령어를 처리하는 로직을 구현합니다.
		// 예를 들어, 서버 상태 확인, 클라이언트 관리 등의 작업을 수행할 수 있습니다.

		DebugLog(Debug::DEBUG_LOG, "Received external command: " + command);
	}
}