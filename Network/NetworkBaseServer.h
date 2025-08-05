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

		// 시스템의 CPU 갯수
		DWORD _cpuCount;

		HANDLE _workerThread[THREAD_MAX];

		DWORD _workThreadCount; // IOCP Worker thread 갯수
		HANDLE _acceptThread;

		bool _useEventThread;
		UINT _fps;

		// Game Update thread handle
		HANDLE _updateThread;
		HANDLE _updateQuitEvent;
		DWORD _updateWaitTime;
		DWORD _updateOldTime;
		DWORD _updateFrame;

		// 게임가드 스레드
		HANDLE _regularScheduleThread;
		HANDLE _regularScheduleQuitEvent;

		Network::OverlappedManager* _overlappedManager;
		Network::ClientManager* _clientManager;
		MessageDispatcher _messageDispatchers[protocol::MESSAGETYPE::MESSAGETYPE_MAX];

	public:
		void Initialize(Network::ClientManager*  clientManager, Network::OverlappedManager* overlappedManager, int serverPort, std::string hostName,int overlappedCount, int maxClient);
		int StartIOCP();

		int StartWorkThreads();
		int StopWorkThreads();

		int StartListenThread();
		int StopListenThread();

		void SetUpdateFrame(DWORD dwFrame);
		int StartUpdateThread();

		int SendMessageToClient(DWORD completionKey, std::shared_ptr<MessageData> messageData);
		Network::NetworkUser* GetNetworkUser(DWORD completionKey);

	public:
		virtual int RegistMessageDispatcher();
		virtual int	WorkProcess() = 0;
		virtual int	AcceptProcess() = 0;
		virtual int	UpdateProcess() = 0;

	protected:
		virtual void DebugLog(Debug::DebugType debugtype, const std::string& message) = 0;
		
	};

	static unsigned int WINAPI WorkThreadProcess(void* pThis)
	{
		/*
		BT_SetTerminate();
		//
		// tbb
		// all threads that uses tbb MUST init the scheduler.
		//
	#ifdef _USE_TBB
		tbb::task_scheduler_init init;
	#endif
	*/
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
		// tbb::global_control ??

		NetworkBaseServer* pTemp = (NetworkBaseServer*)pThis;
		return pTemp->UpdateProcess();
	}
}