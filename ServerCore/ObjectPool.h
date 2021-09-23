#pragma once
#include "MemoryPool.h"


template<typename Type> // ObjectPool�� Ŭ�������� ���� �޸� Ǯ�� �������ִ� ��.
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
	static int32 s_allocSize; //static �������� Ŭ�������� ���������� ObjectPool�� ���ø��̹Ƿ� 
	static MemoryPool s_pool; // ��ǻ� Ŭ���� Ǯ���� ���� �ٸ� static������ ���´�.

};


template<typename Type>
int32 ObjectPool<Type>::s_allocSize = sizeof(Type) + sizeof(MemoryHeader);

template<typename Type>
MemoryPool ObjectPool<Type>::s_pool{ s_allocSize };