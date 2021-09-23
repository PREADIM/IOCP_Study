#pragma once
#include "MemoryPool.h"


template<typename Type> // ObjectPool은 클래스마다 따로 메모리 풀을 가지고있는 것.
class ObjectPool
{
public:
	template<typename... Args>
	static Type* MRent(Args&&... args)
	{
#ifdef _STOMP
		MemoryHeader* ptr = reinterpret_cast<MemoryHeader*>(StompAllocator::Alloc(s_allocSize));
		Type* memory = static_cast<Type*>(MemoryHeader::AttachHeader(ptr, s_allocSize));
#else
		Type* memory = static_cast<Type*>(MemoryHeader::AttachHeader(s_pool.Pop(), s_allocSize));
#endif
		new(memory)Type(forward<Args>(args)...);
		return memory;
	}

	static void MReturn(Type* obj)
	{
		obj->~Type();

#ifdef _STOMP
		StompAllocator::Release(MemoryHeader::DetachHeader(obj));
#else
		s_pool.MReturn(MemoryHeader::DetachHeader(obj));
#endif

	}

	template<typename... Args>
	static shared_ptr<Type> MakeShared(Args&&... args)
	{
		shared_ptr<Type> ptr = { MRent(forward<Args>(args)...), MReturn };
		return ptr;
	}



private:
	static int32 s_allocSize; //static 변수들은 클래스마다 공유하지만 ObjectPool은 템플릿이므로 
	static MemoryPool s_pool; // 사실상 클래스 풀마다 각자 다른 static변수를 갖는다.

};


template<typename Type>
int32 ObjectPool<Type>::s_allocSize = sizeof(Type) + sizeof(MemoryHeader);

template<typename Type>
MemoryPool ObjectPool<Type>::s_pool{ s_allocSize };