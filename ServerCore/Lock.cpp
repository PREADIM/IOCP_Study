#include "pch.h"
#include "Lock.h"
#include "CoreTLS.h"
#include "DeadLockProfiler.h"

void Lock::WriteLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (TL_ThreadId == lockThreadId)
	{
		_writeCount++; // 중요 ! ★★
		return;
	}


	const uint32 beginTick = ::GetTickCount64();
	const uint32 desired = (TL_ThreadId << 16) & WRITE_THREAD_MASK; //
	while (true)
	{
		for (uint32 i = 0; i < MAX_SPIN_COUNT; i++)
		{
			uint32 expected = EMPTY_FLAG;
			if (_lockFlag.compare_exchange_strong(expected, desired))
			{
				_writeCount++; // write 중이다.
				return;
			}
		}
		
		if (::GetTickCount64() - beginTick >= TIMEOUT_TICK)
		{
			cout << name << endl;
			CRASH("LOCK TIMEOUT");
		}

		this_thread::yield();
	}

}

void Lock::WriteUnLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif
	if ((_lockFlag.load() & READ_THREAD_MASK) != 0) // 0이 아니라는것은 READ_THREAD_MASK가 활성화 되어있다는 사실
		CRASH("READING");

	const int32 lockCount = --_writeCount;
	if (lockCount == 0) // 락 카운트가 이제 없으면
		_lockFlag.store(EMPTY_FLAG); //EMPTY
}

void Lock::ReadLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (TL_ThreadId == lockThreadId) // 같은 스레드 라면
	{
		_lockFlag.fetch_add(1); // 중요 ! ★★
		return;
	}


	uint32 beginTick = ::GetTickCount64();
	while (true)
	{
		for (uint32 i = 0; i < MAX_SPIN_COUNT; i++)
		{
			uint32 expected = (_lockFlag.load() & READ_THREAD_MASK); // READ 중인지 아닌지만 판단한다.
			//READ중에 WRITE중이면 안되지만, WRITE중에 READ 하는것은 괜찮다.
			if (_lockFlag.compare_exchange_strong(expected, expected + 1))
				return;
		}

		if (::GetTickCount64() - beginTick >= TIMEOUT_TICK)
			CRASH("TIMEOUT READ");

		this_thread::yield();
	}
}

void Lock::ReadUnLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif
	if ((_lockFlag.fetch_sub(1) & READ_THREAD_MASK) == 0) 
		// atomic의 fetch_sub는 빼기전의 갑을 리턴하는데 해당값이 READ_THREAD_MASK와 &했을때 0이면
		// 이미 _lockFlag는 0이라는 뜻이니깐 무언가 이상하다는 것이다.
		CRASH("MULTIPLE_LOCK");
}
