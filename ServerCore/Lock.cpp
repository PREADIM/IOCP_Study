#include "pch.h"
#include "Lock.h"
#include "CoreTLS.h"

void Lock::WriteLock(const char* name)
{
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (TL_ThreadId == lockThreadId)
		return;

	const uint32 beginTick = ::GetTickCount64();
	const uint32 desired = (TL_ThreadId << 16) & WRITE_THREAD_MASK; //
	while (true)
	{
		for (uint32 i = 0; i < MAX_SPIN_COUNT; i++)
		{
			uint32 expected = EMPTY_FLAG;
			if (_lockFlag.compare_exchange_strong(expected, desired))
			{
				_writeCount++; // write ���̴�.
				return;
			}
		}
		
		if (::GetTickCount64() - beginTick > TIMEOUT_TICK)
		{
			CRASH("LOCK TIMEOUT");
		}

		this_thread::yield();
	}

}

void Lock::WriteUnLock(const char* name)
{
	if ((_lockFlag.load() & READ_THREAD_MASK) != 0) // 0�� �ƴ϶�°��� READ_THREAD_MASK�� Ȱ��ȭ �Ǿ��ִٴ� ���
		CRASH("READING");

	const int32 lockCount = --_writeCount;
	if (lockCount == 0) // �� ī��Ʈ�� ���� ������
		_lockFlag.store(EMPTY_FLAG); //EMPTY
}

void Lock::ReadLock(const char* name)
{
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (TL_ThreadId == lockThreadId) // ���� ������ ���
		return; 

	uint32 beginTick = ::GetTickCount64();
	while (true)
	{
		for (uint32 i = 0; i < MAX_SPIN_COUNT; i++)
		{
			uint32 expected = (_lockFlag.load() & READ_THREAD_MASK); // READ ������ �ƴ����� �Ǵ��Ѵ�.
			//READ�߿� WRITE���̸� �ȵ�����, WRITE�߿� READ �ϴ°��� ������.
			if (_lockFlag.compare_exchange_strong(expected, expected + 1))
				return;
		}

		if (::GetTickCount64() - beginTick > TIMEOUT_TICK)
			CRASH("TIMEOUT READ");

		this_thread::yield();
	}
}

void Lock::ReadUnLock(const char* name)
{
	if ((_lockFlag.fetch_sub(1) & READ_THREAD_MASK) == 0) 
		// atomic�� fetch_sub�� �������� ���� �����ϴµ� �ش簪�� READ_THREAD_MASK�� &������ 0�̸�
		// �̹� _lockFlag�� 0�̶�� ���̴ϱ� ���� �̻��ϴٴ� ���̴�.
		CRASH("MULTIPLE_LOCK");
}
