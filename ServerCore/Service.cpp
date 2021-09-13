#include "pch.h"
#include "Service.h"
#include "Session.h"
#include "Listener.h"



Service::Service(ServiceType type, NetAddress netAddress, IocpCoreRef iocpCore, SessionCreateFunc sessionFunc, int32 maxSessionCount)
	:_type(type), _netAddress(netAddress), _iocpCore(iocpCore), _sessionFunc(sessionFunc), _maxSessionCount(maxSessionCount)
{


}

Service::~Service()
{

}

void Service::CloseService()
{


}

SessionRef Service::CreateSession()
{
	SessionRef session = _sessionFunc(); // 세션 생성함수가 들어있는 함수포인터 실행.

	if (_iocpCore->Register(session) == false)
		return nullptr;


	return session;
}

void Service::AddSession(SessionRef session)
{
	WRITE_LOCK;

	_sessionCount++;
	_sessions.insert(session); // 추가

}

void Service::ReleaseSeession(SessionRef session)
{
	WRITE_LOCK;

	ASSERT_CRASH(_sessions.erase(session) != 0);
	_sessionCount--;
}




ClientService::ClientService(NetAddress netAddress, IocpCoreRef iocpCore, SessionCreateFunc sessionFunc, int32 maxSessionCount)
	:Service(ServiceType::Client, netAddress, iocpCore, sessionFunc , maxSessionCount)
{


}



bool ClientService::Start()
{
	return true;
}




// ------------------------------------------





ServerService::ServerService(NetAddress netAddress, IocpCoreRef iocpCore, SessionCreateFunc sessionFunc, int32 maxSessionCount)
	:Service(ServiceType::Server, netAddress, iocpCore, sessionFunc, maxSessionCount)
{


}



bool ServerService::Start()
{

	if (CanStart() == false)
		return false;

	_listener = MakeShared<Listener>();
	if (_listener == nullptr)
		return false;

	ServerServiceRef service = static_pointer_cast<ServerService>(shared_from_this());
	if (_listener->StartAccept(service) == false)
		return false;


	return true;
}




void ServerService::CloseService()
{
	Service::CloseService();
}
