#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IocpEvent.h"
#include "Session.h"
#include "SocketUtils.h"

Listener::~Listener()
{
	SocketUtils::Close(_socket);

	for (AcceptEvent* acceptEvent : _acceptEvents)
	{
		Xdelete(acceptEvent);
	}
}

HANDLE Listener::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Listener::Dispatch(IocpEvent* iocpEvent, int32 numOfByte) // iocp코어에서 신호가오면 처리하는 함수.
{
	switch (iocpEvent->GetType())
	{
	case EventType::Accept :
		AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent);
		ProcessAccept(acceptEvent); // 미끼를 달아서 낚시대를 던져둔 것을 회수하여 처리하는 함수.
	default:

	}

}

bool Listener::StartAccept(NetAddress netAddress)
{
	_socket = SocketUtils::CreateSocket(); // 소켓 생성
	if (_socket == INVALID_SOCKET)
		return false;

	//Iocp코드와 연동
	if (GIocpCore->Register(shared_from_this()) == false)
		return false;

	//소켓 옵션들 설정
	if (SocketUtils::SetReuseAddress(_socket, true) == false)
		return false;

	if (SocketUtils::SetLinger(_socket, 0, 0) == false)
		return false;

	// 바인드
	if (SocketUtils::Bind(_socket, netAddress) == false)
		return false;

	//리슨
	if (SocketUtils::Listen(_socket) == false)
		return false;


	const int32 accepCount = 1;
	for (int32 i = 0; i < accepCount; i++)
	{
		AcceptEvent* acceptEvent = Xnew<AcceptEvent>();
		_acceptEvents.push_back(acceptEvent);
		RegisterAccept(acceptEvent); // 미끼를 걸어서 낚시대를 던지는 행위. 처리하는 함수가 아님x
	}

}

void Listener::CloseSocket()
{
	SocketUtils::Close(_socket);
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	//SessionRef session = _serverService->CreateSession();

	acceptEvent->Init();
	acceptEvent->_session = session;

	DWORD byteReceived = 0;
	if(false == SocketUtils::AcceptEx(_socket, session->GetSocket(), session->_recvBuffer, 0,
		sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
		&byteReceived, static_cast<LPOVERLAPPED>(acceptEvent)));
	{
		const int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) // WSA_IO_PENDING은 비동기일때 자연스럽게 나타나는 에러
		{
			RegisterAccept(acceptEvent);
		}
	}

	/*BOOL AcceptEx(

			SOCKET sListenSocket,    // ListenSocket

			SOCKET sAcceptSocket,    // (클라이언트 쪽 소켓) 미리 만들어 놓은 소켓, 바인드되거나 연결된 소켓은 안됨.

			PVOID lpOutputBuffer,     // (리시브 버퍼) 소켓의 로컬 , 원격 주소와 첫번째 바이트 스트림을 수신할 버퍼. NULL 불가.

			DWORD dwReceiveDataLength,    //첫번째로 수신할 바이트스트림의 크기, 0이면 수신작업을 수행하지 않는다.

			DWORD dwLocalAddressLength,  //소켓 주소 구조체 + 16

			DWORD dwRemoteAddressLength,  //소켓 주소 구조체 + 16

			LPDWORD lpdwBytesReceived,    //실제로 수신한 바이트수, 동기로 작업했을때만 유효하다.

			LPOVERLAPPED lpOverlapped     //overlapped 구조체 포인터

			);
	*/	



		
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
	SessionRef session = acceptEvent->_session;

	if (false == SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), _socket))
	{
		RegisterAccept(acceptEvent); // 새롭게 낚시대 던지기.
		return;
	}

	SOCKADDR_IN sockAddress;
	int32 sizeOfSockAddr = sizeof(sockAddress);
	if (SOCKET_ERROR == ::getpeername(session->GetSocket(), reinterpret_cast<SOCKADDR*>(&sockAddress), &sizeOfSockAddr));
	{
		RegisterAccept(acceptEvent); // 새롭게 낚시대 던지기.
		return;
	}

	cout << "Client Connect !!" << endl;

	session->SetNetAddress(NetAddress(sockAddress));
	//session->ProcessConnect();
	RegisterAccept(acceptEvent);

}
