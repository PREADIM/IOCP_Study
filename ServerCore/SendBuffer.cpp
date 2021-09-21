#include "pch.h"
#include "SendBuffer.h"


SendBuffer::SendBuffer(int32 bufferSize)
{
	_buffer.resize(bufferSize);
}

SendBuffer::~SendBuffer()
{
}

void SendBuffer::CopyData(void* data, int32 len)
{
	ASSERT_CRASH(Capacity() >= len); // 버퍼의 크기가 더 큰경우
	::memcpy(_buffer.data(), data, len);
	_writeSize = len; // send할 크기
}