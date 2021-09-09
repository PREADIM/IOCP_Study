#pragma once
#include "NetAddress.h"


class SocketUtils
{
public:
	// WSAIoctl�� ���� �Լ� ������ ������.
	static LPFN_CONNECTEX ConnectEx;
	static LPFN_ACCEPTEX AcceptEx;
	static LPFN_DISCONNECTEX DisconnectEx;


public:
	// ù �ʱ�ȭ 
	static void Init();
	static void Clear();

	//���� LPFN_ ���ĵ鿡 �Լ��� ���ε� ���� �Լ�.
	static bool BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* LPFN_fn); 
	// ���� ����
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