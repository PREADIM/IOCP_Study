#pragma once

#define OUT


//LOCK






//Crash

#define CRASH(cause)						\
{											\
	uint32* crash = nullptr;				\
	__analysis_assume(crash != nullptr);	\
	*crash = 0xDEADBEEF;					\
}												//crash�� nullptr�� �ƴϿ��� ������ ��������


#define ASSERT_CRASH(expr)			\
{									\
	if(!(expr))						\
	{								\
		CRASH("ASSERT_CHRASH");		\
		__analysis_assume(expr);	\
	}								\
}									








