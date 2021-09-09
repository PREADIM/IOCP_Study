#pragma once
#include "NetAddress.h"


class SocketUtils
{
public:
	// WSAIoctl에 넣을 함수 포인터 변수들.
	static LPFN_CONNECTEX ConnectEx;
	static LPFN_ACCEPTEX AcceptEx;
	static LPFN_DISCONNECTEX DisconnectEx;


public:
	// 첫 초기화 
	static void Init();
	static void Clear();

	//위의 LPFN_ 형식들에 함수를 바인딩 해줄 함수.
	static bool BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* LPFN_fn); 
	// 소켓 생성
	static SOCKET CreateSocket(); 
	static bool Bind(SOCKET socket, NetAddress netAddr);
	static bool BindServerAnyAddress(SOCKET socket, uint16 port);
	static bool Listen(SOCKET socket, int32 backLog = SOMAXCONN);
	static void Close(SOCKET& socket);

	static bool SetLinger(SOCKET socket, uint16 onoff, uint16 linger);
	static bool SetReuseAddress(SOCKET socket, bool flag);
	static bool SetRecvBufferSize(SOCKET socket, int32 size);
	static bool SetSendBufferSize(SOCKET socket, int32 size);
	static bool SetTcpNoDelay(SOCKET socket, bool flag);
	static bool SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);

};


template<typename T>
static bool SetSockOption(SOCKET socket, int32 level, int32 optName, T optVal)
{
	return SOCKET_ERROR != ::setsockopt(socket, level, optName, reinterpret_cast<char*>(&optVal), sizeof(T));
}