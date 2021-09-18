#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"
#include "IocpEvent.h"

Session::Session()
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}



void Session::Send(BYTE* buffer, int32 len)
{
	SendEvent* sendEvent = Xnew<SendEvent>();
	sendEvent->owner = shared_from_this();
	sendEvent->buffer.resize(len);
	::memcpy(sendEvent->buffer.data(), buffer, len);

	WRITE_LOCK;
	RegisterSend(sendEvent);
}



// ��κ� ��Ȳ������ ������ ���� ����.
// ���� ���� �Ҷ��� Ŭ�� ������ �پ �۵��ϱ⶧���� ������ ����.
// ������ �������� ������ ���� ����⶧���� �ۼ�.
// �� ������ �̿��ؼ� ���� ������ �ٴ� �۾��� �Ѵ�.
bool Session::Connect()
{

	return RegisterConnect();

}




void Session::Disconnect(const WCHAR* cause)
{
	if (_connected.exchange(false) == false)
		return;

	wcout << "Disconnect : " << cause << endl;

	OnDisconnected(); // �����ε�
	GetService()->ReleaseSeession(GetSessionRef());

	RegisterDisconnect();
}


HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}




// IocpCore���� GetQueueCompletionStatus���� ��ȣ�� ���� ����� �Լ�.
void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
	switch (iocpEvent->_eventType)
	{
	case EventType::Connect:
		ProcessConnect();
		break;
	case EventType::Recv:
		ProcessRecv(numOfBytes);
		break;
	case EventType::Send:
		ProcessSend(static_cast<SendEvent*>(iocpEvent), numOfBytes);
		break;
	case EventType::Disconnect:
		ProcessDisconnect();
		break;
	default:
		break;
	}

}

bool Session::RegisterConnect()
{
	if (IsConnected())
		return false;

	if (GetService()->GetServiceType() != ServiceType::Client) // Ŭ���̾�Ʈ ������ �ƴϸ� �ȵ�.
		return false;

	if (SocketUtils::SetReuseAddress(_socket, true) == false)
		return false;

	if (SocketUtils::BindServerAnyAddress(_socket, 0/*���°�*/) == false)
		return false;

	_connectEvent.Init();
	_connectEvent.owner = shared_from_this();


	DWORD numOfBytes = 0;
	SOCKADDR_IN sockAddr = GetService()->GetNetAddress().GetSockAddr(); 
	// Ŭ���̾�Ʈ ���� �̱⶧���� ���⿡ �ִ� NetAddress�� �������� �ǹ��Ѵ�. ������ NetAddress

	if (false == SocketUtils::ConnectEx(_socket, reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockAddr),
		nullptr, 0, &numOfBytes, &_connectEvent))
	{
		int32 errCode = WSAGetLastError();
		if (errCode != WSA_IO_PENDING) // ���
		{
			_connectEvent.owner = nullptr;
			return false;
		}
	}


	return true;

}



bool Session::RegisterDisconnect()
{
	_disconnectEvent.Init();
	_disconnectEvent.owner = shared_from_this();


	// DisconnectEx�� ���ڿ� TF_REUSE_SOCKET�� ������ �ش� ������ �ٽ� AcceptEx�� ConnectEx���� ����Ҽ� �ְ��Ѵ�.
	if (false == SocketUtils::DisconnectEx(_socket, &_disconnectEvent, TF_REUSE_SOCKET, 0))
	{
		int32 errCode = WSAGetLastError();
		if (errCode != WSA_IO_PENDING)
		{
			_disconnectEvent.owner = nullptr; //RELEASE REF
			return false;
		}
	}

	return true;
}





void Session::RegisterRecv()
{
	if (IsConnected() == false)
		return;

	_recvEvent.Init();
	_recvEvent.owner = shared_from_this(); // ADD_REF 
	// �ϴ������� GetQueueCompletionPort���� OVERLAPPED�� ��ӹ��� IocpEventŬ�����ȿ� owner�� IocpObject�� �Ǵ��ϱ� ���ؼ�.

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer);
	wsaBuf.len = len32(_recvBuffer);

	DWORD numOfBytes = 0;
	DWORD flags = 0;
	if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, &numOfBytes, &flags, &_recvEvent, nullptr))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) // ����� �񵿱�ó������ �������� �����̴�.
		{
			HandleError(errorCode);
			_recvEvent.owner = nullptr; // RELEASE_REF
		}
	}
}

void Session::RegisterSend(SendEvent* sendEvent)
{
	if (IsConnected() == false)
		return;

	WSABUF wsaBuf;
	wsaBuf.buf = (char*)sendEvent->buffer.data();
	wsaBuf.len = (ULONG)sendEvent->buffer.size();

	DWORD numOfBytes = 0;
	if (SOCKET_ERROR == ::WSASend(_socket, &wsaBuf, 1, &numOfBytes, 0, sendEvent, nullptr))
	{
		if (int32 errorCode = ::WSAGetLastError())
		{
			if (errorCode != WSA_IO_PENDING)
			{
				HandleError(errorCode);
				sendEvent->owner = nullptr; // ���۷��� ���ֱ�
				Xdelete(sendEvent);
			}
		}
	}


}


//Ŭ�� �������� �������� �� �Լ��� ����, ������ Ŭ���������� �ٸ� ������ �������� �� �Լ��� ����.
void Session::ProcessConnect() 
{
	_connectEvent.owner = nullptr;

	_connected.store(true); // ����Ǿ���.

	GetService()->AddSession(GetSessionRef()); // ���񽺿� ���ǵ��

	//������ �ڵ忡�� ������.
	OnConnected();

	RegisterRecv(); // recv�� �ϱ����� ���ô� �̳��� ������.

}




void Session::ProcessDisconnect()
{

	_disconnectEvent.owner = nullptr;

}




void Session::ProcessRecv(int32 numOfBytes)
{
	_recvEvent.owner = nullptr; 
	// ���⼭ RegisterRecv���� ���� shared_from_this()�� nullptr�� RELEASE ����������, ���� ���������ʴ´�.

	if (numOfBytes == 0)
	{
		Disconnect(L"Recv 0");
		return;
	}

	OnRecv(_recvBuffer, numOfBytes);

	RegisterRecv(); // �ٽ� recv �̳��� ������.

}

void Session::ProcessSend(SendEvent* sendEvent, int32 numOfBytes)
{
	sendEvent->owner = nullptr;
	Xdelete(sendEvent);

	if (numOfBytes == 0)
	{
		Disconnect(L"Send");
		return;
	}

	//������ �ڵ忡�� �����ε�.
	OnSend(numOfBytes);

}

void Session::HandleError(int32 errorCode)
{
	switch (errorCode)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED: // ������ ����.
		Disconnect(L"HandleError");
		break;

	default:
		cout << "Handle Error" << endl;
		break;
	}
}
