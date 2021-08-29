#include "pch.h"
#include "ThreadManager.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"

ThreadManager::ThreadManager()
{
}

ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::InitTLS()
{
	static Atomic<uint32> SThreadId = 1;
	TL_ThreadId = SThreadId.fetch_add(1); //������ ���ÿ� �ִ� ������ ���̵� 1������
}

void ThreadManager::DestoryTLS()
{
}

void ThreadManager::Launch(function<void(void)> callback)
{
	thread_vector.push_back(thread([=]()
		{
			InitTLS();
			callback();
		}));
	
}

void ThreadManager::Join()
{
	for (thread& _thread : thread_vector)
	{
		if (_thread.joinable())
		{
			_thread.join();
		}
	}
	thread_vector.clear();
}
