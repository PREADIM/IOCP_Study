#include "pch.h"
#include "Allocator.h"
#include "MemoryManager.h"


// ★ BaseAllocator ★ //

void* BaseAllocator::Alloc(int32 size)
{
	return ::malloc(size);
}

void BaseAllocator::Reserve(void* ptr)
{

	::free(ptr);

}


// ★ StompAllocator ★ //

void* StompAllocator::Alloc(int32 size)
{
	int64 pageCount = (PAGE_SIZE - 1) + size / PAGE_SIZE;
	int64 dataOffset = pageCount * PAGE_SIZE - size;
	void* baseAddress = ::VirtualAlloc(NULL, pageCount * PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	return static_cast<void*>(static_cast<int8*>(baseAddress) + dataOffset);
	//데이터를 사이즈 만큼 뒤로 밀어서 뒤부터 작성함 (메모리 오버플로를 막기위해)
}

void StompAllocator::Release(void* ptr)
{
	const int64 address = reinterpret_cast<int64>(ptr);
	const int64 baseAddress = address % PAGE_SIZE;
	::VirtualFree(reinterpret_cast<void*>(baseAddress), 0, MEM_RELEASE);

}


// ★ PoolAllocator ★ //


void* PoolAllocator::Alloc(int32 size)
{
	return GMemoryManager->Allocate(size);
}

void PoolAllocator::Release(void* ptr)
{
	GMemoryManager->Release(ptr);
}


