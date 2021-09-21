#pragma once
#include "IocpCore.h"
#include "NetAddress.h"

class AcceptEvent;
class ServerService;

//using SessionCreateFunc = function<SessionRef(void)>

class Listener : public IocpObject // iocp�� �Ѿ� �� �ֵ��� Dispatch �ϱ����� ���
{
public:
	Listener() = default;
	~Listener();

public:
	virtual HANDLE GetHandle();
	virtual void Dispatch(class IocpEvent* iocpEvnet, int32 numOfByte = 0);

public:
	bool StartAccept(ServerServiceRef service);
	void CloseSocket();

private:
	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* accepEvent);


protected:
	SOCKET _socket = INVALID_SOCKET;
	Vector<AcceptEvent*> _acceptEvents;
	ServerServiceRef _serverService = nullptr; // �ʿ��� �۾��� �� �������ִ� ��.


	/*
	set<SessionRef> _sessions;
	int32 _sessionCount;
	IocpCoreRef _iocpCore;
	SessionCreateFunc _sessionFunc;
	NetAddress _netAddress;*/

};

