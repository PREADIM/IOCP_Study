#include "pch.h"
#include "MemoryPool.h"


MemoryPool::MemoryPool(int32 size) : _allocSize(size)
{
	::InitializeSListHead(&_SLIST_Header);
}


MemoryPool::~MemoryPool()
{
	while (MemoryHeader* memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&_SLIST_Header)))
	{
		::_aligned_free(memory);
	}

	// static_cast를 함과 동시에 nullptr인지 확인할수있는 문법
}


void MemoryPool::MReturn(MemoryHeader* ptr) // 메모리 풀에 반납하는 함수.
{
	ptr->_allocSize = 0;

	::InterlockedPushEntrySList(&_SLIST_Header, static_cast<PSLIST_ENTRY>(ptr));
	// 메모리 반납

	_useCount.fetch_sub(1); // 사용중 횟수 차감.
	_reserveCount.fetch_add(1);
}


MemoryHeader* MemoryPool::MRent() // 메모리 풀에서 빌리는 함수.
{
	MemoryHeader* memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&_SLIST_Header));

	if (memory == nullptr)
	{
		memory = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(_allocSize, SLIST_ALIGNMENT));
		//::_aligned_malloc은 size와 정렬 바이트를 인자로 넘겨주면된다.
	}
	else
	{
		ASSERT_CRASH(memory->_allocSize == 0);
		_reserveCount.fetch_add(1);
	}

	_useCount.fetch_add(1);

	return memory;
}