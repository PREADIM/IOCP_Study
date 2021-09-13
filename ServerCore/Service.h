#pragma once

#include "NetAddress.h"
#include "IocpCore.h"
#include "Listener.h"
#include <functional>


enum class ServiceType : uint8
{
	Server,
	Client
};


using SessionCreateFunc = function<SessionRef(void)>;


class Service : public enable_shared_from_this<Service>
{
public:
	Service(ServiceType type, NetAddress netAddress, IocpCoreRef iocpCore, SessionCreateFunc sessionFunc, int32 maxSessionCount = 1);
	virtual ~Service();


	virtual bool Start() abstract;
	virtual void CloseService();

	bool CanStart() { return _sessionFunc != nullptr; }
	void SetSessionFunc(SessionCreateFunc func) { _sessionFunc = func; }

	SessionRef CreateSession();
	void AddSession(SessionRef session);
	void ReleaseSeession(SessionRef session);
	int32 GetCurrentSessionCount() { return _sessionCount; }
	int32 GetMaxSessionCount() { return _maxSessionCount; }
	
public:
	ServiceType GetServiceType() { return _type; }
	NetAddress GetNetAddress() { return _netAddress;}
	IocpCoreRef& GetIocpCore() { return _iocpCore; }



protected : //멤버 변수들
	USE_LOCK;

	ServiceType _type;
	NetAddress _netAddress = {};
	IocpCoreRef _iocpCore;

	Set<SessionRef> _sessions; // 지금까지 연결한 세션의 갯수 (세션엔 클라이언트의 정보가 담겨져있음)
	int32 _sessionCount = 0;
	int32 _maxSessionCount = 0;
	SessionCreateFunc _sessionFunc;

};






class ClientService : public Service
{
public:
	ClientService(NetAddress netAddress, IocpCoreRef iocpCore, SessionCreateFunc sessionFunc, int32 maxSessionCount = 1);
	virtual ~ClientService() {}

	virtual bool Start() override;

};




class ServerService : public Service
{
public:
	ServerService(NetAddress netAddress, IocpCoreRef iocpCore, SessionCreateFunc sessionFunc, int32 maxSessionCount = 1);
	virtual ~ServerService() {}

	virtual bool Start() override;
	virtual void CloseService();

private:
	ListenerRef _listener = nullptr;

};