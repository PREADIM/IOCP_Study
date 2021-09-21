#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"
#include "RecvBuffer.h"


class Service;

class Session : public IocpObject
{
	friend class Listener;
	friend class IocpCore;
	friend class Service;

	enum
	{
		BUFFER_SIZE = 0x10000
	};


public :
	Session();
	virtual ~Session();


public:
	void				Send(SendBufferRef sendBuffer);
	bool				Connect();
	void				Disconnect(const WCHAR* cause);

	shared_ptr<Service>	GetService() { return _service.lock(); } //lock은 weak_ptr을 shared_ptr로 만든다.
	void				SetService(shared_ptr<Service> service) { _service = service; }

public:
	/* 정보 관련 */
	void				SetNetAddress(NetAddress address) { _netAddress = address; }
	NetAddress			GetAddress() { return _netAddress; }
	SOCKET				GetSocket() { return _socket; }
	bool				IsConnected() { return _connected; }
	SessionRef			GetSessionRef() { return static_pointer_cast<Session>(shared_from_this()); }

public:
	/* 인터페이스 구현 */
	virtual HANDLE GetHandle();
	virtual void Dispatch(class IocpEvent* iocpEvnet, int32 numOfBytes = 0);


private:
	/* 전송 관련 */
	// 미끼 던지기
	bool				RegisterConnect();
	bool				RegisterDisconnect();
	void				RegisterRecv();
	void				RegisterSend();


	// 처리
	void				ProcessConnect();
	void				ProcessDisconnect();
	void				ProcessRecv(int32 numOfBytes);
	void				ProcessSend(int32 numOfBytes);

	void				HandleError(int32 errorCode);


protected:
	/* 컨텐츠 코드에서 오버로딩 */
	virtual void		OnConnected() { }
	virtual int32		OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void		OnSend(int32 len) { }
	virtual void		OnDisconnected() { }



private:
	weak_ptr<Service> _service; //weak_ptr은 shared_ptr처럼 쓸수있지만 레퍼런스 카운팅이 증가되지않는다.

	SOCKET _socket = INVALID_SOCKET;
	NetAddress _netAddress = {};
	Atomic<bool> _connected = false;

private:
	USE_LOCK;

	/* 수신 관련 */

	RecvBuffer _recvBuffer;

	/* 송신 관련 */
	Queue<SendBufferRef> _sendQueue;
	Atomic<bool> _sendRegister = false; // RegisterSend 중인가?

private:
	/* IocpEvent 재사용 */

	ConnectEvent		_connectEvent;
	DisconnectEvent		_disconnectEvent;
	RecvEvent			_recvEvent;
	SendEvent			_sendEvent;

};


