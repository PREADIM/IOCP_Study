#pragma once

// �� BaseAllocator �� //

class BaseAllocator
{
public :
	void* Alloc(int32 size);
	void Reserve(void* ptr);
};


// �� StompAllocator �� //

class StompAllocator
{
	enum { PAGE_SIZE = 0x1000 };

public:
	void* Alloc(int32 size);
	void Reserve(void* ptr);
};


// �� PoolAllocator �� //


class PoolAllocator
{
public:
	void* Alloc(int32 size);
	void Release(void* ptr);
};



//�� STLAllocator ��//

template<typename T>
class STLAllocator
{
	using Type_Value = T;

public:
	T* allocate(size_t count)
	{
		const int32 allocSize = static_cast<int32>(count + sizeof(T));
		return static_cast<T*>(PoolAllocator::Alloc(allocSize));
	}

	void deallocate(T* ptr, size_t count)
	{
		PoolAllocator::Release(ptr);
	}

	template<typename U>
	bool operator== (const STLAllocator<U>&) { return true; }

	template<typename U>
	bool operator!= (const STLAllocator<U>&) { return false; }

};