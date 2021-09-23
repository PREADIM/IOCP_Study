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

	// static_cast�� �԰� ���ÿ� nullptr���� Ȯ���Ҽ��ִ� ����
}


void MemoryPool::MReturn(MemoryHeader* ptr) // �޸� Ǯ�� �ݳ��ϴ� �Լ�.
{
	ptr->_allocSize = 0;

	::InterlockedPushEntrySList(&_SLIST_Header, static_cast<PSLIST_ENTRY>(ptr));
	// �޸� �ݳ�

	_useCount.fetch_sub(1); // ����� Ƚ�� ����.
	_reserveCount.fetch_add(1);
}


MemoryHeader* MemoryPool::MRent() // �޸� Ǯ���� ������ �Լ�.
{
	MemoryHeader* memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&_SLIST_Header));

	if (memory == nullptr)
	{
		memory = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(_allocSize, SLIST_ALIGNMENT));
		//::_aligned_malloc�� size�� ���� ����Ʈ�� ���ڷ� �Ѱ��ָ�ȴ�.
	}
	else
	{
		ASSERT_CRASH(memory->_allocSize == 0);
		_reserveCount.fetch_add(1);
	}

	_useCount.fetch_add(1);

	return memory;
}