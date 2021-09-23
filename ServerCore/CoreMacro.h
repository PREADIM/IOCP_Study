#pragma once

#define OUT


//LOCK

#define USE_MANY_LOCK(count)	Lock _locks[count];
#define USE_LOCK				USE_MANY_LOCK(1)
#define READ_LOCK_IDX(idx)		ReadLockGuard readLockGuard_##idx(_locks[idx], typeid(this).name());
#define READ_LOCK				READ_LOCK_IDX(0)
#define WRITE_LOCK_IDX(idx)		WriteLockGuard writeLockGuard_##idx(_locks[idx], typeid(this).name());
#define WRITE_LOCK				WRITE_LOCK_IDX(0)



// Memory


#ifdef _DEBUG
#define Xalloc(size) PoolAllocator::Alloc(size);
#define XRelease(ptr) PoolAllocator::Release(size);
#else
#define Xalloc(size) BaseAllocator::Alloc(size);
#define XRelease(ptr) BaseAllocator::Release(size);
#endif





//Crash

#define CRASH(cause)						\
{											\
	uint32* crash = nullptr;				\
	__analysis_assume(crash != nullptr);	\
	*crash = 0xDEADBEEF;					\
}												//crash가 nullptr이 아니여도 오류를 내지마라


#define ASSERT_CRASH(expr)			\
{									\
	if(!(expr))						\
	{								\
		CRASH("ASSERT_CHRASH");		\
		__analysis_assume(expr);	\
	}								\
}									



#define _STOMP




