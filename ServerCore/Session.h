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

	shared_ptr<Service>	GetService() { return _service.lock(); } //lock�� weak_ptr�� shared_ptr�� �����.
	void				SetService(shared_ptr<Service> service) { _service = service; }

public:
	/* ���� ���� */
	void				SetNetAddress(NetAddress address) { _netAddress = address; }
	NetAddress			GetAddress() { return _netAddress; }
	SOCKET				GetSocket() { return _socket; }
	bool				IsConnected() { return _connected; }
	SessionRef			GetSessionRef() { return static_pointer_cast<Session>(shared_from_this()); }

public:
	/* �������̽� ���� */
	virtual HANDLE GetHandle();
	virtual void Dispatch(class IocpEvent* iocpEvnet, int32 numOfBytes = 0);


private:
	/* ���� ���� */
	// �̳� ������
	bool				RegisterConnect();
	bool				RegisterDisconnect();
	void				RegisterRecv();
	void				RegisterSend();


	// ó��
	void				ProcessConnect();
	void				ProcessDisconnect();
	void				ProcessRecv(int32 numOfBytes);
	void				ProcessSend(int32 numOfBytes);

	void				HandleError(int32 errorCode);


protected:
	/* ������ �ڵ忡�� �����ε� */
	virtual void		OnConnected() { }
	virtual int32		OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void		OnSend(int32 len) { }
	virtual void		OnDisconnected() { }



private:
	weak_ptr<Service> _service; //weak_ptr�� shared_ptró�� ���������� ���۷��� ī������ ���������ʴ´�.

	SOCKET _socket = INVALID_SOCKET;
	NetAddress _netAddress = {};
	Atomic<bool> _connected = false;

private:
	USE_LOCK;

	/* ���� ���� */

	RecvBuffer _recvBuffer;

	/* �۽� ���� */
	Queue<SendBufferRef> _sendQueue;
	Atomic<bool> _sendRegister = false; // RegisterSend ���ΰ�?

private:
	/* IocpEvent ���� */

	ConnectEvent		_connectEvent;
	DisconnectEvent		_disconnectEvent;
	RecvEvent			_recvEvent;
	SendEvent			_sendEvent;

};


