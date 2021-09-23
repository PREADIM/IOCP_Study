#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"
#include "IocpEvent.h"

Session::Session() : _recvBuffer(BUFFER_SIZE)
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}



void Session::Send(SendBufferRef sendBuffer)
{
	// ���� RegisterSend�� �ɸ��� ���� ���¶�� , �ɾ��ش�.
	WRITE_LOCK;

	_sendQueue.push(sendBuffer); // �Ϸ������� ���������� ť�� ����ؼ� ���δ�.

	if (_sendRegistered.exchange(true) == false) // �Ϸ� ����.
		RegisterSend();

	cout << "Test" << endl;

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
	GetService()->ReleaseSession(GetSessionRef());

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
		ProcessSend(numOfBytes);
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
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
	wsaBuf.len = _recvBuffer.FreeSize(); // �ִ�� �������ִ� ������ ũ��

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

void Session::RegisterSend()
{
	if (IsConnected() == false)
		return;

	_sendEvent.Init();
	_sendEvent.owner = shared_from_this(); // ADD_REF 


	{
		WRITE_LOCK;

		int32 writeSize = 0;
		while (_sendQueue.empty() == false)
		{
			SendBufferRef sendBuffer = _sendQueue.front();

			writeSize += sendBuffer->WriteSize();

			_sendQueue.pop();
			_sendEvent._sendBuffers.push_back(sendBuffer); 
			// sendQueue�� �ִ� sendBuffer�� sendEvent�ȿ� �ִ� sendBufferRef�� �ִ´�.  sendQueue�� empty�ɶ�����
		}
	}

	//Scatter-Gather : ����� �ִ� �����͵��� ��Ƽ� �� �濡 ������.
	Vector<WSABUF> wsaBufs;
	wsaBufs.reserve(_sendEvent._sendBuffers.size()); // sendBufferRef�� �� ����.
	for (SendBufferRef sendBuffer : _sendEvent._sendBuffers)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
		wsaBuf.len = static_cast<LONG>(sendBuffer->WriteSize());
		wsaBufs.push_back(wsaBuf);
	}
	//WSABUF�� �ѹ��� ��Ƽ� WSASend�Ѵ�.


	DWORD numOfBytes = 0;

	if (SOCKET_ERROR == ::WSASend(_socket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), &numOfBytes, 0, &_sendEvent, nullptr))
	{
		if (int32 errorCode = ::WSAGetLastError())
		{
			if (errorCode != WSA_IO_PENDING)
			{
				HandleError(errorCode);
				_sendEvent.owner = nullptr; // ���۷��� ���ֱ�
				_sendEvent._sendBuffers.clear();
				_sendRegistered.store(false); // Register�� �����ʴ� ����.
			}
		}
	}

	cout << "Test2" << endl;

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

	if (_recvBuffer.OnWrite(numOfBytes) == false) // Recv�� �������̴� _writePos ����.
	{
		// �����������
		Disconnect(L"OnWrite Overflow");
		return;
	}

	int32 dataSize = _recvBuffer.DataSize(); // �������� ���� �����Ϳ� �̹��� ���� �������� ���� �� ������.
	int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize);
	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
	{
		// false �ϰ��
		Disconnect(L"OnRead Overflow");
		return;
	}


	//Ŀ�� ����
	_recvBuffer.Clean();


	RegisterRecv(); // �ٽ� recv �̳��� ������.

}

void Session::ProcessSend(int32 numOfBytes)
{
	_sendEvent.owner = nullptr; //RELEASE_PTR
	_sendEvent._sendBuffers.clear(); // vector �ʱ�ȭ.

	if (numOfBytes == 0)
	{
		Disconnect(L"Send");
		return;
	}

	//������ �ڵ忡�� �����ε�.
	OnSend(numOfBytes);
	

	cout << "Test3" << endl;

	WRITE_LOCK;
	if (_sendQueue.empty()) // ������ִٴ°� send�� �����͸� �� send�ߴٴ� ���̰�, ���� �����ִٴ� �� send�� �����Ͱ� �����ִٴ� ��.
		_sendRegistered.store(false);
	else
		RegisterSend(); // ���� send�� �����Ͱ� ���������� �ٽ� �õ�.



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
