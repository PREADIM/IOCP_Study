#include "pch.h"
#include "SocketUtils.h"


LPFN_CONNECTEX SocketUtils::ConnectEx = nullptr;
LPFN_ACCEPTEX SocketUtils::AcceptEx = nullptr;
LPFN_DISCONNECTEX SocketUtils::DisconnectEx = nullptr;


void SocketUtils::Init()
{
	WSADATA wsaData;
	ASSERT_CRASH(::WSAStartup(MAKEWORD(2, 2), &wsaData) == 0);

	SOCKET dummySocket = CreateSocket();
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_CONNECTEX, reinterpret_cast<LPVOID*>(&ConnectEx)));
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_ACCEPTEX, reinterpret_cast<LPVOID*>(&AcceptEx)));
	ASSERT_CRASH(BindWindowsFunction(dummySocket, WSAID_DISCONNECTEX, reinterpret_cast<LPVOID*>(&DisconnectEx)));

	Close(dummySocket); // 잠시 사용했던 소켓 다시 초기화.
}

void SocketUtils::Clear()
{
}

bool SocketUtils::BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* LPFN_fn)
{
	DWORD bytes = 0;
	return ::WSAIoctl(socket, SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guid, sizeof(guid), 
		LPFN_fn, sizeof(LPFN_fn), 
		&bytes, NULL, NULL);	
}

SOCKET SocketUtils::CreateSocket()
{
	return ::WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	//WSA_FLAG_OVERLAPPED를 켜주면 우리가 사용하는 OVERLAPPED 계열 함수들을 사용할수있다.
	/* 기본적으로 있는::socket함수에서 따로 WSA_FLAG_OVERLAPPED를 하지않았는데도 사용할수 있었던이유는,
	 애초에 기본적으로 켜져있기때문.*/
}

bool SocketUtils::Bind(SOCKET socket, NetAddress netAddr)
{
	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&netAddr.GetSockAddr()), sizeof(SOCKADDR_IN));

}

bool SocketUtils::BindServerAnyAddress(SOCKET socket, uint16 port) // 서버용.
{
	SOCKADDR_IN myAddress;
	myAddress.sin_family = AF_INET;
	myAddress.sin_addr.s_addr = ::htonl(INADDR_ANY);
	myAddress.sin_port = ::htons(port);

	return SOCKET_ERROR != ::bind(socket, reinterpret_cast<const SOCKADDR*>(&myAddress), sizeof(myAddress));
}

bool SocketUtils::Listen(SOCKET socket, int32 backLog)
{

	return SOCKET_ERROR != ::listen(socket, backLog);
}

void SocketUtils::Close(SOCKET& socket)
{
	if (socket != INVALID_SOCKET)
		::closesocket(socket);
	socket = INVALID_SOCKET;
}

bool SocketUtils::SetLinger(SOCKET socket, uint16 onoff, uint16 linger)
{
	LINGER option;
	option.l_linger = linger;
	option.l_onoff = onoff;

	return SetSockOption(socket, SOL_SOCKET, SO_LINGER, option);
}

bool SocketUtils::SetReuseAddress(SOCKET socket, bool flag)
{
	return SetSockOption(socket, SOL_SOCKET, SO_REUSEADDR, flag);
}

bool SocketUtils::SetRecvBufferSize(SOCKET socket, int32 size)
{
	return SetSockOption(socket, SOL_SOCKET, SO_RCVBUF, size);
}

bool SocketUtils::SetSendBufferSize(SOCKET socket, int32 size)
{
	return SetSockOption(socket, SOL_SOCKET, SO_SNDBUF, size);
}

bool SocketUtils::SetTcpNoDelay(SOCKET socket, bool flag)
{
	return SetSockOption(socket, SOL_SOCKET, TCP_NODELAY, flag);
}

bool SocketUtils::SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket)
{
	return SetSockOption(socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, listenSocket);
}





