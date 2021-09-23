#pragma once



extern thread_local uint32 TL_ThreadId;
extern thread_local std::stack<int32> TL_LockStack;
extern thread_local SendBufferChunkRef TL_SendBufferChunk;

