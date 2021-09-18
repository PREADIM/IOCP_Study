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
	SessionRef session = _sessionFunc(); // ���� �����Լ��� ����ִ� �Լ������� ����.
	session->SetService(shared_from_this()); // ���ǿ� service ���

	if (_iocpCore->Register(session) == false) //Iocp�� �����Ѵ�.
		return nullptr;


	return session;
}

void Service::AddSession(SessionRef session)
{
	WRITE_LOCK;

	_sessionCount++;
	_sessions.insert(session); // �߰�

}

void Service::ReleaseSeession(SessionRef session)
{
	WRITE_LOCK;

	ASSERT_CRASH(_sessions.erase(session) != 0);
	_sessionCount--;
}




// --------------------------------------------------------



ClientService::ClientService(NetAddress netAddress, IocpCoreRef iocpCore, SessionCreateFunc sessionFunc, int32 maxSessionCount)
	:Service(ServiceType::Client, netAddress, iocpCore, sessionFunc , maxSessionCount)
{


}



bool ClientService::Start()
{
	if (CanStart() == false)
		return false;

	const int32 sessionCount = GetMaxSessionCount();
	for (int32 i = 0; i < sessionCount; i++)
	{
		SessionRef session = CreateSession();
		if (session->Connect() == false)
			return false;
	}

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
	{
		cout << " Can Error" << endl;
		return false;
	}


	_listener = MakeShared<Listener>();
	if (_listener == nullptr)
	{
		cout << "Error" << endl;
		return false;
	}
		

	ServerServiceRef service = static_pointer_cast<ServerService>(shared_from_this());
	if (_listener->StartAccept(service) == false)
	{
		cout << "service Error" << endl;
		return false;
	}


	return true;
}




void ServerService::CloseService()
{
	Service::CloseService();
}
