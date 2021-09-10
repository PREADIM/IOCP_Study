#pragma once

class MemoryPool;


class MemoryManager
{
	enum
	{
		// ~1024까지 32단위, ~2048까지 128단위 , ~4096 까지 256단위
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
	ojb->~Type();
	PoolAllocator::Release(obj);
}


template<typename Type, typename... Args>
shared_ptr<Type> MakeShared(Args&&... args)
{
	return shared_ptr<Type>{ Xnew<Type>(forwared<Args>(args)...), Xdelete<Type> };
}