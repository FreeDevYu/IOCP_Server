#include <iostream>

#include "../Network/pch.h"
#include "../Network/OverlappedManager.h""
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>

#include "../ThirdParty/flatbuffers/flatbuffers.h"
#include "../Network/MESSAGE_PROTOCOL_generated.h"
#include "../Network/MessageBuilder.h"

Network::MessageBuilder messageBuilder;
Network::OverlappedManager* overlappedManager;
SOCKET Socket = INVALID_SOCKET;
HANDLE IocpHandle;

HANDLE WorkThread;

void ReceiveReady();
void MessageSend(std::shared_ptr<Network::MessageData> messageData);

static unsigned int WINAPI Work(void* pThis);

int main()
{
	std::string ip = "127.0.0.1";
	int port = 9090;


	WSADATA wsaData;
	LPFN_CONNECTEX connectEx = NULL;
	GUID connectExGuid = WSAID_CONNECTEX;
	sockaddr_in serverAddr;
	sockaddr_in localAddr;
	DWORD bytes;

	overlappedManager = new Network::OverlappedManager();
	overlappedManager->Initialize(30);//temporary value

	IocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	int tempResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (tempResult != 0)
	{
		std::cout << "WSAStartup failed with error code: " << tempResult << std::endl;
		return 0;
	}

	DWORD dwThreadId;
	WorkThread = (HANDLE) ::_beginthreadex(
		NULL,
		0,
		Work,
		nullptr,
		0,
		(unsigned int*)&dwThreadId);

	Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (WSAIoctl(Socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&connectExGuid, sizeof(connectExGuid),
		&connectEx, sizeof(connectEx),
		&bytes, NULL, NULL))
	{
		std::cout << "WSAIoctl failed with error code: " << WSAGetLastError() << std::endl;
		//Utility::LogError("Client", "ClientManager", "WSAIoctl 실패: " + std::to_string(WSAGetLastError()));
		closesocket(Socket);
		return 0;
	}

	CreateIoCompletionPort((HANDLE)Socket, IocpHandle, (ULONG_PTR)0, 0);

	// 서버 주소 설정
	serverAddr = { 0 };
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

	// 로컬 주소 바인딩
	localAddr = { 0 };
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = INADDR_ANY;
	localAddr.sin_port = 0;  // 자동 할당

	if (bind(Socket, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR)
	{
		std::cout << "Bind failed with error code: " << WSAGetLastError() << std::endl;
		//Utility::LogError("Client", "ClientManager", "클라이언트 bind 실패: " + std::to_string(WSAGetLastError()));
		//losesocket(*targetSocket);
		return false;
	}

	auto overlapped = overlappedManager->Pop(Network::OperationType::OP_DEFAULT);

	BOOL result = connectEx(Socket, (sockaddr*)&serverAddr, sizeof(serverAddr), NULL, 0, NULL, &*overlapped);
	if (!result)
	{
		int errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			std::cout << "ConnectEx failed with error code: " << errorCode << std::endl;
			//Utility::LogError("Network", "Client", "ConnectEx 실패! 오류 코드: " + std::to_string(errorCode));

			// 소켓을 안전하게 닫고 정리
			if (Socket != INVALID_SOCKET)
			{
				closesocket(Socket);
				Socket = INVALID_SOCKET;
			}
			WSACleanup();
			return 0;
		}
	}

	std::cout << "Connecting to server at " << ip << ":" << port << "..." << std::endl;

	while (true)
	{

	}

	return 0;
}

void MessageSend(std::shared_ptr<Network::MessageData> messageData)
{
	DWORD flags = 0;
	DWORD sendSize = messageData->OverlappedSize();
	int resultCode = 0;

	auto overlapped = overlappedManager->Pop(Network::OperationType::OP_SEND);
	overlapped->CopyFromMessageData(*messageData);

	resultCode = ::WSASend(Socket,
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
			std::cout << "WSASend failed with error code: " << resultCode << std::endl;
			return;
		}
	}
	std::cout << "Message sent successfully." << std::endl;
}
void ReceiveReady()
{
	auto overlapped = overlappedManager->Pop(Network::OperationType::OP_RECV);

	int errorCode;
	DWORD flags = 0;
	int result = WSARecv(Socket,
		&overlapped->Wsabuf,
		1,
		NULL,
		&flags,
		overlapped,
		NULL);

	std::string log;
	errorCode = WSAGetLastError();
	if (result == SOCKET_ERROR && errorCode != WSA_IO_PENDING)
	{
		std::cout << "WSARecv failed with error code: " << errorCode << std::endl;
		//log = "WSARecv 실패! 오류 코드: " + std::to_string(errorCode);
		//Utility::LogError("Network", "Client", log);
		return;
	}

	std::cout << "Socket Receive Ready" << std::endl;
}

static unsigned int WINAPI Work(void* pThis)
{
	DWORD bytesTransferred;
	ULONG_PTR completionKey;
	Network::CustomOverlapped* overlapped = nullptr;

	Network::MessageHeader header{ 0,0 };
	char* bodyBuffer = nullptr;

	while (true)
	{
		bytesTransferred = 0;
		completionKey = 0;
		overlapped = nullptr;

		bool result = GetQueuedCompletionStatus(IocpHandle, &bytesTransferred, &completionKey, reinterpret_cast<LPOVERLAPPED*>(&overlapped), INFINITE); 

		if (!result)
		{
			int errorCode = WSAGetLastError();
			//Utility::LogError("Network", "Session", "GetQueuedCompletionStatus Failed! Error Code: " + std::to_string(errorCode));

			switch (errorCode)
			{
			case WSAECONNRESET:
			case WSAECONNABORTED:
			case WSAENETRESET:
			case WSAETIMEDOUT:
			case WSAENOTCONN:
			case WSAESHUTDOWN:
			case ERROR_NETNAME_DELETED:
			case ERROR_CONNECTION_ABORTED:
				//_clientMap.unsafe_erase(completionKey);
				//_disconnectCallback(_serverType, completionKey, bytesTransferred, errorCode, overlapped);
				break;
			default:
				break;
			}

			continue;
		}
		else if (result)
		{
			auto targetOverlapped = static_cast<Network::CustomOverlapped*>(overlapped);

			switch (targetOverlapped->OperationType)
			{

			case Network::OperationType::OP_DEFAULT:
			{
				// 연결이 완료되었을 때
				std::cout << "Connected to server successfully!" << std::endl;
				ReceiveReady();

				flatbuffers::FlatBufferBuilder builder;
				std::string serverName = "TestServer";
				auto serverNameOffset = builder.CreateString(serverName);
				builder.Finish(protocol::CreateREQUEST_REGISTER(builder, serverNameOffset));

				Network::MessageHeader header(builder.GetSize(), protocol::MESSAGETYPE::MESSAGETYPE_REQUEST_REGISTER);
				std::shared_ptr<Network::MessageData> messageData = std::make_shared<Network::MessageData>(
					completionKey,
					header,
					(char*)builder.GetBufferPointer()
				);

				MessageSend(messageData);
			
				break;
			}

			case Network::OperationType::OP_RECV:
			{
				if (bytesTransferred <= 0)
				{
					//_disconnectCallback(_serverType, completionKey, bytesTransferred, 0, overlapped);
					continue;
				}

				ReceiveReady();

				messageBuilder.InsertMessage(overlapped->Wsabuf.buf, bytesTransferred);

				int havingMessage = messageBuilder.MessageCheckAndReturn(header, bodyBuffer);
				
				if (havingMessage == 1)
				{
					int contents = static_cast<protocol::MESSAGETYPE>(header.ContentsType);

					switch (contents)
					{
					case protocol::MESSAGETYPE::MESSAGETYPE_RESPONSE_REGISTER:
					{
						auto responseRegister = flatbuffers::GetRoot<protocol::RESPONSE_REGISTER>(bodyBuffer);

						if (responseRegister == nullptr)
						{
							std::cout << "Invalid RESPONSE_REGISTER message." << std::endl;
							break;
						}

						bool success = responseRegister->feedback();
						if (success)
						{
							std::cout << "Registration successful." << std::endl;
						}
						else
						{
							std::cout << "Registration failed." << std::endl;
						}
						break;
					}

					case protocol::MESSAGETYPE::MESSAGETYPE_REQUEST_HEARTBEAT:
					{
						auto responseRegister = flatbuffers::GetRoot<protocol::REQUEST_HEARTBEAT>(bodyBuffer);
						if (responseRegister == nullptr)
						{
							std::cout << "Invalid REQUEST_HEARTBEAT message." << std::endl;
							break;
						}

						std::cout << "Received REQUEST_HEARTBEAT message." << std::endl;

						flatbuffers::FlatBufferBuilder builder;
						builder.Finish(protocol::CreateRESPONSE_HEARTBEAT(builder));

						Network::MessageHeader header(builder.GetSize(), protocol::MESSAGETYPE::MESSAGETYPE_RESPONSE_HEARTBEAT);
						std::shared_ptr<Network::MessageData> messageData = std::make_shared<Network::MessageData>(
							completionKey,
							header,
							(char*)builder.GetBufferPointer()
						);

						//MessageSend(messageData); -> 오프라인테스트용 주석처리
						builder.Clear();
						break;
					}

					std::cout << "Received message of type: " << contents << std::endl;
					}
				}
				break;
			}

			case Network::OperationType::OP_SEND:
			{
				break;
			}

			}
		}

		overlapped->Clear();
		overlappedManager->Push(overlapped);
	}
}