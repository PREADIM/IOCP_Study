#include "pch.h"
#include "MemoryManager.h"
#include "MemoryPool.h"

MemoryManager::MemoryManager()
{
	int32 index = 0;
	int32 tableIndex = 0;

	for (int32 size = 32; size <= 1024; size += 32) //1024 까지는 32단위
	{
		MemoryPool* pool = new MemoryPool(size); // 32단위 사이즈를 가지고있는 메모리 풀 생성
		_pools.push_back(pool);

		while (tableIndex <= size)
		{
			_poolTable[tableIndex] = pool;
			tableIndex++;
		}
	}

	for (int32 size = 128; size <= 2048; size += 128) //2048 까지는 128단위
	{
		MemoryPool* pool = new MemoryPool(size); // 32단위 사이즈를 가지고있는 메모리 풀 생성
		_pools.push_back(pool);

		while (tableIndex <= size)
		{
			_poolTable[tableIndex] = pool;
			tableIndex++;
		}
	}

	for (int32 size = 256; size <= 4096; size += 256) //4096 까지는 256단위
	{
		MemoryPool* pool = new MemoryPool(size); // 32단위 사이즈를 가지고있는 메모리 풀 생성
		_pools.push_back(pool);

		while (tableIndex <= size)
		{
			_poolTable[tableIndex] = pool;
			tableIndex++;
		}
	}
}

MemoryManager::~MemoryManager()
{
	for (MemoryPool* pool : _pools)
	{
		delete pool;
	}

	_pools.clear();

}

void* MemoryManager::Allocate(int32 size)
{
	MemoryHeader* header = nullptr;
	const int32 allocSize = size + sizeof(MemoryHeader);

#ifdef  _STOMP
	header = reinterpret_cast<MemoryHeader*>(StompAllocator::Alloc(allocSize));
#else
	if (allocSize > MAX_ALLOC_SIZE) // 사이즈가 맥스 ALLOC보다 크면 메모리풀에 만드는것보다 따로만드는게 좋다.
	{
		header = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(allocSize, SLIST_ALIGNMENT));
	}
	else
	{
		header = _poolTable[allocSize]->MRent(); // MAX_ALLOC보다 작으면 그냥 메모리풀에서 가져온다.
	}
#endif //  _STOMP

	return MemoryHeader::AttachHeader(header, allocSize);
}

void MemoryManager::Release(void* ptr)
{
	MemoryHeader* header = MemoryHeader::DetachHeader(ptr);

	const int32 allocSize = header->_allocSize;
	ASSERT_CRASH(allocSize > 0); // allocSize가 0보다 작은거면 이상한 것이므로 크래시

#ifdef _STOMP
	StompAllocator::Release(header);
#else
	if (allocSize > MAX_ALLOC_SIZE) // 사이즈가 맥스 ALLOC보다 크면 메모리풀에 만드는것보다 따로만드는게 좋다.
	{
		::_aligned_free(header);
	}
	else
	{
		_poolTable[allocSize]->MReturn(header); // 다시 돌려준다.
	}

#endif 

}
