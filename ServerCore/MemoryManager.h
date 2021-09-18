#pragma once
#include "Allocator.h"

class MemoryPool;

class MemoryManager
{
	enum
	{
		// ~1024���� 32����, ~2048���� 128���� , ~4096 ���� 256����
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
		MAX_ALLOC_SIZE = 4096
	};

public:
	MemoryManager();
	~MemoryManager();


	void* Allocate(int32 size);
	void Release(void* ptr);


private:
	vector<MemoryPool*> _pools;

	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1];

};




template<typename Type, typename... Args>
Type* Xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(PoolAllocator::Alloc(sizeof(Type)));
	new(memory) Type(forward<Args>(args)...);
	return memory;
}


template<typename Type>
void Xdelete(Type* obj)
{
	obj->~Type();
	PoolAllocator::Release(obj);
}


template<typename Type, typename... Args>
shared_ptr<Type> MakeShared(Args&&... args)
{
	return shared_ptr<Type>{ Xnew<Type>(forward<Args>(args)...), Xdelete<Type> };
}