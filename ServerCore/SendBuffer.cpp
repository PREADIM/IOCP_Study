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
	_writeSize = writeSize; // ������ �� ����
	_owner->Close(writeSize);
}



// -------------------------------------



// SendBufferManager���� SendBufferChunk�� TLS�������� ����ϱ⶧���� 
// ��Ƽ ���������� �ƴ� �̱� ������ ������� �����ص� �ȴ�. ���� LOCK�� ���ʿ����.


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
		TL_SendBufferChunk = MRent(); // �޸� �Ҵ�
		TL_SendBufferChunk->Reset();
	}
		

	ASSERT_CRASH(TL_SendBufferChunk->IsOpen() == false);

	//�� ���� ���ŷ� ��ü.
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
		if (_sendBufferChunks.empty() == false) // ����ûũ Ǯ�� �����Ͱ� ������,
		{
			SendBufferChunkRef sendBufferChunk = _sendBufferChunks.back();
			_sendBufferChunks.pop_back();
			return sendBufferChunk;
		}
	}
	

	return SendBufferChunkRef(Xnew<SendBufferChunk>(), PushGlobal); 
	//delete �Ҷ� �޸𸮸� �����°� �ƴ� PushGlobal ȣ��.
}



void SendBufferManager::MRetrun(SendBufferChunkRef buffer)
{
	WRITE_LOCK;
	_sendBufferChunks.push_back(buffer);

}


void SendBufferManager::PushGlobal(SendBufferChunk* buffer) //static�� ���������� ����.
{

	GSendBufferManager->MRetrun(SendBufferChunkRef(buffer, PushGlobal));

}

