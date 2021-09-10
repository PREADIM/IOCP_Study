#pragma once
#include "IocpCore.h"
#include "NetAddress.h"

class AcceptEvent;


class Listener : public IocpObject // iocp에 넘어 온 애들을 Dispatch 하기위해 상속
{
public:
	Listener() = default;
	~Listener();

public:
	virtual HANDLE GetHandle();
	virtual void Dispatch(class IocpEvent* iocpEvnet, int32 numOfByte = 0);

public:
	bool StartAccept(NetAddress netAddress);
	void CloseSocket();

private:
	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* accepEvent);


protected:
	SOCKET _socket = INVALID_SOCKET;
	Vector<AcceptEvent*> _acceptEvents;
	//ServerServiceRef _serverService;

};

