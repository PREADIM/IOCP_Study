#include "pch.h"
#include "CoreGlobal.h"
#include "ThreadManager.h"
#include "MemoryManager.h"

ThreadManager* GThreadManager = nullptr;
MemoryManager* GMemoryManager = nullptr;

class CoreGlobal
{
public:
	CoreGlobal()
	{
		GThreadManager = new ThreadManager;
		GMemoryManager = new MemoryManager;
	}
	~CoreGlobal()
	{
		delete GThreadManager;
		delete GMemoryManager;
	}
}GCoreGlobal;