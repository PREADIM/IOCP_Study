#pragma once

#define OUT


//LOCK






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








