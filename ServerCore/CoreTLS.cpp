#include "pch.h"
#include "CoreTLS.h"


thread_local uint32 TL_ThreadId=0;
thread_local std::stack<int32> TL_LockStack;
thread_local SendBufferChunkRef TL_SendBufferChunk;