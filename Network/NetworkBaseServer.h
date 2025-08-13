#pragma once
#include "pch.h"
#include "Lock.h"
#include <winsock2.h>
#include <ws2tcpip.h> 
#include <functional>
#include <format>
#include "ClientManager.h"
#include "OverlappedManager.h"
#include "NetworkUser.h"
#include "NetworkDefine.h"

#include "../ThirdParty/flatbuffers/flatbuffers.h"
#include "MESSAGE_PROTOCOL_generated.h"

namespace Network
{
	class NetworkBaseServer : public DefaultLock
	{
		struct MessageDispatcher
		{
			std::function<void(Network::NetworkBaseServer&, std::shared_ptr<Network::MessageData>)> ProtocolFunction;

			MessageDispatcher()
			{
				ProtocolFunction = NULL;
			}
		};
		

	public:
		NetworkBaseServer();
		virtual ~NetworkBaseServer() = 0;

	protected:
		volatile bool _serverOn;

		HANDLE _iocpHandle;
		SOCKET _serverSocket;

		int _serverPort;
		std::string _hostName;
		int _maxClient;

		// �ý����� CPU ����
		DWORD _cpuCount;

		HANDLE _workerThread[THREAD_MAX];

		DWORD _workThreadCount; // IOCP Worker thread ����
		HANDLE _acceptThread;

		bool _useEventThread;
		UINT _fps;

		// Game Update thread handle
		HANDLE _updateThread;
		HANDLE _updateQuitEvent;
		DWORD _updateWaitTime;
		DWORD _updateOldTime;
		DWORD _updateFrame;

		// ���Ӱ��� ������
		HANDLE _regularScheduleThread;
		HANDLE _regularScheduleQuitEvent;

		Network::OverlappedManager* _overlappedManager;
		Network::ClientManager* _clientManager;
		MessageDispatcher _messageDispatchers[protocol::MESSAGETYPE::MESSAGETYPE_MAX];

	protected:
		void Initialize(int maxClient, int overlappedCount);
		int StartIOCP();

		int StartWorkThreads();
		int StopWorkThreads();

		int StartListenThread();
		int StopListenThread();

		void SetUpdateFrame(DWORD dwFrame);
		int StartUpdateThread();

		int SendMessageToClient(DWORD completionKey, std::shared_ptr<MessageData> messageData);
		Network::NetworkUser* GetNetworkUser(DWORD completionKey);

		virtual int DisconnectClient(DWORD completionKey);

	public:
		virtual int PowerOnSequence() = 0;
		virtual int RegistMessageDispatcher();
		virtual int	WorkProcess() = 0;
		virtual int	AcceptProcess() = 0;
		virtual int	UpdateProcess() = 0;

	protected:
		virtual void DebugLog(Debug::DebugType debugtype, const std::string& message) = 0;
		virtual void ReceiveExternalCommand(std::string& command) = 0;
	};

	static unsigned int WINAPI WorkThreadProcess(void* pThis)
	{
		NetworkBaseServer* pTemp = (NetworkBaseServer*)pThis;
		return pTemp->WorkProcess();
	}

	static unsigned int WINAPI AcceptThreadProcess(void* pThis)
	{
		NetworkBaseServer* pTemp = (NetworkBaseServer*)pThis;
		return pTemp->AcceptProcess();
	}

	static unsigned int WINAPI UpdateThreadProcess(void* pThis)
	{
		NetworkBaseServer* pTemp = (NetworkBaseServer*)pThis;
		return pTemp->UpdateProcess();
	}
}