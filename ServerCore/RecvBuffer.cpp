#include "pch.h"
#include "RecvBuffer.h"

RecvBuffer::RecvBuffer(int32 bufferSize) : _bufferSize(bufferSize)
{
	_capacity = bufferSize * BUFFER_COUNT;
	_buffer.resize(_capacity);

}

RecvBuffer::~RecvBuffer()
{


}

void RecvBuffer::Clean()
{
	int32 dataSize = DataSize();
	if (dataSize == 0)
	{
		//딱 마침 읽기+쓰기 커서가 동일한 위치면 리셋.
		_readPos = _writePos = 0;
	}
	else
	{
		// 여유공간이 버퍼 1개크기 미만이면 , 데이터 앞으로 땡김.
		if (FreeSize() < _bufferSize)
		{
			::memcpy(&_buffer[0], &_buffer[_readPos], dataSize);
			_readPos = 0;
			_writePos = dataSize;
		}
	}
}

bool RecvBuffer::OnRead(int32 numOfBytes)
{
	if (numOfBytes > DataSize()) // numOfByte가 데이터 크기보다 큰경우. 여기서 numOfBytes는 읽을 데이터 수이다.
		return false;

	_readPos += numOfBytes;
	return true;
}

bool RecvBuffer::OnWrite(int32 numOfBytes)
{
	if (numOfBytes > FreeSize()) // numOfByte가 남은 데이터 크기보다 큰경우. 여기서 numOfBytes는 읽을 데이터 수이다.
		return false;

	_writePos += numOfBytes;
	return true;
}
