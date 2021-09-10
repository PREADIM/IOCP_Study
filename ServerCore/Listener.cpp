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

void Listener::Dispatch(IocpEvent* iocpEvent, int32 numOfByte) // iocp�ھ�� ��ȣ������ ó���ϴ� �Լ�.
{
	switch (iocpEvent->GetType())
	{
	case EventType::Accept :
		AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent);
		ProcessAccept(acceptEvent); // �̳��� �޾Ƽ� ���ô븦 ������ ���� ȸ���Ͽ� ó���ϴ� �Լ�.
	default:

	}

}

bool Listener::StartAccept(NetAddress netAddress)
{
	_socket = SocketUtils::CreateSocket(); // ���� ����
	if (_socket == INVALID_SOCKET)
		return false;

	//Iocp�ڵ�� ����
	if (GIocpCore->Register(shared_from_this()) == false)
		return false;

	//���� �ɼǵ� ����
	if (SocketUtils::SetReuseAddress(_socket, true) == false)
		return false;

	if (SocketUtils::SetLinger(_socket, 0, 0) == false)
		return false;

	// ���ε�
	if (SocketUtils::Bind(_socket, netAddress) == false)
		return false;

	//����
	if (SocketUtils::Listen(_socket) == false)
		return false;


	const int32 accepCount = 1;
	for (int32 i = 0; i < accepCount; i++)
	{
		AcceptEvent* acceptEvent = Xnew<AcceptEvent>();
		_acceptEvents.push_back(acceptEvent);
		RegisterAccept(acceptEvent); // �̳��� �ɾ ���ô븦 ������ ����. ó���ϴ� �Լ��� �ƴ�x
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
		if (errorCode != WSA_IO_PENDING) // WSA_IO_PENDING�� �񵿱��϶� �ڿ������� ��Ÿ���� ����
		{
			RegisterAccept(acceptEvent);
		}
	}

	/*BOOL AcceptEx(

			SOCKET sListenSocket,    // ListenSocket

			SOCKET sAcceptSocket,    // (Ŭ���̾�Ʈ �� ����) �̸� ����� ���� ����, ���ε�ǰų� ����� ������ �ȵ�.

			PVOID lpOutputBuffer,     // (���ú� ����) ������ ���� , ���� �ּҿ� ù��° ����Ʈ ��Ʈ���� ������ ����. NULL �Ұ�.

			DWORD dwReceiveDataLength,    //ù��°�� ������ ����Ʈ��Ʈ���� ũ��, 0�̸� �����۾��� �������� �ʴ´�.

			DWORD dwLocalAddressLength,  //���� �ּ� ����ü + 16

			DWORD dwRemoteAddressLength,  //���� �ּ� ����ü + 16

			LPDWORD lpdwBytesReceived,    //������ ������ ����Ʈ��, ����� �۾��������� ��ȿ�ϴ�.

			LPOVERLAPPED lpOverlapped     //overlapped ����ü ������

			);
	*/	



		
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
	SessionRef session = acceptEvent->_session;

	if (false == SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), _socket))
	{
		RegisterAccept(acceptEvent); // ���Ӱ� ���ô� ������.
		return;
	}

	SOCKADDR_IN sockAddress;
	int32 sizeOfSockAddr = sizeof(sockAddress);
	if (SOCKET_ERROR == ::getpeername(session->GetSocket(), reinterpret_cast<SOCKADDR*>(&sockAddress), &sizeOfSockAddr));
	{
		RegisterAccept(acceptEvent); // ���Ӱ� ���ô� ������.
		return;
	}

	cout << "Client Connect !!" << endl;

	session->SetNetAddress(NetAddress(sockAddress));
	//session->ProcessConnect();
	RegisterAccept(acceptEvent);

}
