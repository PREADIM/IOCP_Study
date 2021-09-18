#include "pch.h"
#include "MemoryManager.h"
#include "MemoryPool.h"

MemoryManager::MemoryManager()
{
	int32 index = 0;
	int32 tableIndex = 0;

	for (int32 size = 32; size <= 1024; size += 32) //1024 ������ 32����
	{
		MemoryPool* pool = new MemoryPool(size); // 32���� ����� �������ִ� �޸� Ǯ ����
		_pools.push_back(pool);

		while (tableIndex <= size)
		{
			_poolTable[tableIndex] = pool;
			tableIndex++;
		}
	}

	for (int32 size = 128; size <= 2048; size += 128) //2048 ������ 128����
	{
		MemoryPool* pool = new MemoryPool(size); // 32���� ����� �������ִ� �޸� Ǯ ����
		_pools.push_back(pool);

		while (tableIndex <= size)
		{
			_poolTable[tableIndex] = pool;
			tableIndex++;
		}
	}

	for (int32 size = 256; size <= 4096; size += 256) //4096 ������ 256����
	{
		MemoryPool* pool = new MemoryPool(size); // 32���� ����� �������ִ� �޸� Ǯ ����
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
	if (allocSize > MAX_ALLOC_SIZE) // ����� �ƽ� ALLOC���� ũ�� �޸�Ǯ�� ����°ͺ��� ���θ���°� ����.
	{
		header = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(allocSize, SLIST_ALIGNMENT));
	}
	else
	{
		header = _poolTable[allocSize]->MRent(); // MAX_ALLOC���� ������ �׳� �޸�Ǯ���� �����´�.
	}
#endif //  _STOMP

	return MemoryHeader::AttachHeader(header, allocSize);
}

void MemoryManager::Release(void* ptr)
{
	MemoryHeader* header = MemoryHeader::DetachHeader(ptr);

	const int32 allocSize = header->_allocSize;
	ASSERT_CRASH(allocSize > 0); // allocSize�� 0���� �����Ÿ� �̻��� ���̹Ƿ� ũ����

#ifdef _STOMP
	StompAllocator::Release(header);
#else
	if (allocSize > MAX_ALLOC_SIZE) // ����� �ƽ� ALLOC���� ũ�� �޸�Ǯ�� ����°ͺ��� ���θ���°� ����.
	{
		::_aligned_free(header);
	}
	else
	{
		_poolTable[allocSize]->MReturn(header); // �ٽ� �����ش�.
	}

#endif 

}
