#include "pch.h"
#include "SendBuffer.h"


SendBuffer::SendBuffer(SendBufferChunkRef owner, BYTE* buffer, int32 allocSize)
	:_owner(owner) , _buffer(buffer), _allocSize(allocSize)
{
	
}

SendBuffer::~SendBuffer()
{
}

void SendBuffer::Close(uint32 writeSize)
{
	ASSERT_CRASH(_allocSize >= writeSize); 
	_writeSize = writeSize; // 데이터 수 저장
	_owner->Close(writeSize);
}



// -------------------------------------



// SendBufferManager에서 SendBufferChunk는 TLS영역으로 사용하기때문에 
// 멀티 스레드방식이 아닌 싱글 스레드 방식으로 설계해도 된다. 굳이 LOCK을 걸필요없음.


SendBufferChunk::SendBufferChunk()
{


}



SendBufferChunk::~SendBufferChunk()
{
}



void SendBufferChunk::Reset()
{
	_open = false;
	_usedSize = 0;

}



SendBufferRef SendBufferChunk::Open(uint32 allocSize)
{
	ASSERT_CRASH(allocSize <= SEND_BUFFER_CHUNK_SIZE);
	ASSERT_CRASH(_open == false);

	if (allocSize > FreeSize())
		return nullptr;

	_open = true;

	return ObjectPool<SendBuffer>::MakeShared(shared_from_this(), Buffer(), allocSize);

}



void SendBufferChunk::Close(uint32 writeSize)
{
	ASSERT_CRASH(_open == true);
	_open = false;
	_usedSize += writeSize;

}



// ----------------------------------



SendBufferRef SendBufferManager::Open(uint32 size)
{
	if (TL_SendBufferChunk == nullptr)
	{
		TL_SendBufferChunk = MRent(); // 메모리 할당
		TL_SendBufferChunk->Reset();
	}
		

	ASSERT_CRASH(TL_SendBufferChunk->IsOpen() == false);

	//다 쓰면 새거로 교체.
	if (TL_SendBufferChunk->FreeSize() < size)
	{
		TL_SendBufferChunk = MRent();
		TL_SendBufferChunk->Reset();
	}



	cout << "FREE : " << TL_SendBufferChunk->FreeSize() << endl;

	return TL_SendBufferChunk->Open(size);

}


SendBufferChunkRef SendBufferManager::MRent()
{
	{
		WRITE_LOCK;
		if (_sendBufferChunks.empty() == false) // 센드청크 풀에 데이터가 있으면,
		{
			SendBufferChunkRef sendBufferChunk = _sendBufferChunks.back();
			_sendBufferChunks.pop_back();
			return sendBufferChunk;
		}
	}
	

	return SendBufferChunkRef(Xnew<SendBufferChunk>(), PushGlobal); 
	//delete 할때 메모리를 날리는게 아닌 PushGlobal 호출.
}



void SendBufferManager::MRetrun(SendBufferChunkRef buffer)
{
	WRITE_LOCK;
	_sendBufferChunks.push_back(buffer);

}


void SendBufferManager::PushGlobal(SendBufferChunk* buffer) //static이 붙은이유는 위에.
{

	GSendBufferManager->MRetrun(SendBufferChunkRef(buffer, PushGlobal));

}

