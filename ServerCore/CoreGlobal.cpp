#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "MemoryManager.h"
#include "IocpCore.h"
#include "SocketUtils.h"
#include "DeadLockProfiler.h"

ThreadManager* GThreadManager = nullptr;
MemoryManager* GMemoryManager = nullptr;
DeadLockProfiler* GDeadLockProfiler = nullptr;
SendBufferManager* GSendBufferManager = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		GThreadManager = new ThreadManager;
		GMemoryManager = new MemoryManager;
		GDeadLockProfiler = new DeadLockProfiler;
		GSendBufferManager = new SendBufferManager;
		SocketUtils::Init();
	}
	~CoreGlobal()
	{
		delete GThreadManager;
		delete GMemoryManager;
		delete GDeadLockProfiler;
		SocketUtils::Clear();
	}
}GCoreGlobal;