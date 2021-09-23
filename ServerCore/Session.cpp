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
	// 현재 RegisterSend가 걸리지 않은 상태라면 , 걸어준다.
	WRITE_LOCK;

	_sendQueue.push(sendBuffer); // 완료통지가 되지않으면 큐에 계속해서 쌓인다.

	if (_sendRegistered.exchange(true) == false) // 완료 통지.
		RegisterSend();

	cout << "Test" << endl;

}



// 대부분 상황에서는 연결할 일은 없다.
// 서버 역할 할때는 클라가 서버에 붙어서 작동하기때문에 할일이 없다.
// 하지만 서버끼리 연결할 일이 생기기때문에 작성.
// 이 세션을 이용해서 상대방 서버로 붙는 작업을 한다.
bool Session::Connect()
{

	return RegisterConnect();

}




void Session::Disconnect(const WCHAR* cause)
{
	if (_connected.exchange(false) == false)
		return;

	wcout << "Disconnect : " << cause << endl;

	OnDisconnected(); // 오버로딩
	GetService()->ReleaseSession(GetSessionRef());

	RegisterDisconnect();
}


HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}




// IocpCore에서 GetQueueCompletionStatus에서 신호가 오면 실행될 함수.
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

	if (GetService()->GetServiceType() != ServiceType::Client) // 클라이언트 역할이 아니면 안됨.
		return false;

	if (SocketUtils::SetReuseAddress(_socket, true) == false)
		return false;

	if (SocketUtils::BindServerAnyAddress(_socket, 0/*남는거*/) == false)
		return false;

	_connectEvent.Init();
	_connectEvent.owner = shared_from_this();


	DWORD numOfBytes = 0;
	SOCKADDR_IN sockAddr = GetService()->GetNetAddress().GetSockAddr(); 
	// 클라이언트 역할 이기때문에 여기에 있는 NetAddress는 서버쪽을 의미한다. 서버의 NetAddress

	if (false == SocketUtils::ConnectEx(_socket, reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockAddr),
		nullptr, 0, &numOfBytes, &_connectEvent))
	{
		int32 errCode = WSAGetLastError();
		if (errCode != WSA_IO_PENDING) // 펜딩
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


	// DisconnectEx의 인자에 TF_REUSE_SOCKET을 넣으면 해당 소켓을 다시 AcceptEx나 ConnectEx에서 사용할수 있게한다.
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
	// 하는이유는 GetQueueCompletionPort에서 OVERLAPPED를 상속받은 IocpEvent클래스안에 owner로 IocpObject를 판단하기 위해서.

	WSABUF wsaBuf;
	wsaBuf.buf = reinterpret_cast<char*>(_recvBuffer.WritePos());
	wsaBuf.len = _recvBuffer.FreeSize(); // 최대로 받을수있는 버퍼의 크기

	DWORD numOfBytes = 0;
	DWORD flags = 0;
	if (SOCKET_ERROR == ::WSARecv(_socket, &wsaBuf, 1, &numOfBytes, &flags, &_recvEvent, nullptr))
	{
		int32 errorCode = ::WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) // 펜딩은 비동기처리에서 정상적인 오류이다.
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
			// sendQueue에 있던 sendBuffer를 sendEvent안에 있는 sendBufferRef에 넣는다.  sendQueue가 empty될때까지
		}
	}

	//Scatter-Gather : 흩어져 있는 데이터들을 모아서 한 방에 보낸다.
	Vector<WSABUF> wsaBufs;
	wsaBufs.reserve(_sendEvent._sendBuffers.size()); // sendBufferRef의 총 갯수.
	for (SendBufferRef sendBuffer : _sendEvent._sendBuffers)
	{
		WSABUF wsaBuf;
		wsaBuf.buf = reinterpret_cast<char*>(sendBuffer->Buffer());
		wsaBuf.len = static_cast<LONG>(sendBuffer->WriteSize());
		wsaBufs.push_back(wsaBuf);
	}
	//WSABUF를 한번에 모아서 WSASend한다.


	DWORD numOfBytes = 0;

	if (SOCKET_ERROR == ::WSASend(_socket, wsaBufs.data(), static_cast<DWORD>(wsaBufs.size()), &numOfBytes, 0, &_sendEvent, nullptr))
	{
		if (int32 errorCode = ::WSAGetLastError())
		{
			if (errorCode != WSA_IO_PENDING)
			{
				HandleError(errorCode);
				_sendEvent.owner = nullptr; // 레퍼런스 없애기
				_sendEvent._sendBuffers.clear();
				_sendRegistered.store(false); // Register를 하지않는 상태.
			}
		}
	}

	cout << "Test2" << endl;

}


//클라가 서버한테 붙을때도 이 함수를 쓰고, 서버가 클라입장으로 다른 서버에 붙을때도 이 함수를 쓴다.
void Session::ProcessConnect() 
{
	_connectEvent.owner = nullptr;

	_connected.store(true); // 연결되엇다.

	GetService()->AddSession(GetSessionRef()); // 서비스에 세션등록

	//컨텐츠 코드에서 재정의.
	OnConnected();

	RegisterRecv(); // recv를 하기위한 낚시대 미끼를 던진다.

}




void Session::ProcessDisconnect()
{

	_disconnectEvent.owner = nullptr;

}




void Session::ProcessRecv(int32 numOfBytes)
{
	_recvEvent.owner = nullptr; 
	// 여기서 RegisterRecv에서 해준 shared_from_this()를 nullptr로 RELEASE 하지않으면, 영영 삭제되지않는다.

	if (numOfBytes == 0)
	{
		Disconnect(L"Recv 0");
		return;
	}

	if (_recvBuffer.OnWrite(numOfBytes) == false) // Recv를 받은것이니 _writePos 증가.
	{
		// 실패했을경우
		Disconnect(L"OnWrite Overflow");
		return;
	}

	int32 dataSize = _recvBuffer.DataSize(); // 지난번에 받은 데이터와 이번에 받은 데이터의 현재 총 사이즈.
	int32 processLen = OnRecv(_recvBuffer.ReadPos(), dataSize);
	if (processLen < 0 || dataSize < processLen || _recvBuffer.OnRead(processLen) == false)
	{
		// false 일경우
		Disconnect(L"OnRead Overflow");
		return;
	}


	//커서 정리
	_recvBuffer.Clean();


	RegisterRecv(); // 다시 recv 미끼를 던진다.

}

void Session::ProcessSend(int32 numOfBytes)
{
	_sendEvent.owner = nullptr; //RELEASE_PTR
	_sendEvent._sendBuffers.clear(); // vector 초기화.

	if (numOfBytes == 0)
	{
		Disconnect(L"Send");
		return;
	}

	//컨텐츠 코드에서 오버로딩.
	OnSend(numOfBytes);
	

	cout << "Test3" << endl;

	WRITE_LOCK;
	if (_sendQueue.empty()) // 비어져있다는건 send할 데이터를 다 send했다는 뜻이고, 아직 남아있다는 건 send할 데이터가 남아있다는 뜻.
		_sendRegistered.store(false);
	else
		RegisterSend(); // 아직 send할 데이터가 남아있으니 다시 시도.



}

void Session::HandleError(int32 errorCode)
{
	switch (errorCode)
	{
	case WSAECONNRESET:
	case WSAECONNABORTED: // 연결이 끊김.
		Disconnect(L"HandleError");
		break;

	default:
		cout << "Handle Error" << endl;
		break;
	}
}
