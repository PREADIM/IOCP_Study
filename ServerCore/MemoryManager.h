#pragma once

class MemoryPool;


class MemoryManager
{
	enum
	{
		// ~1024까지 32단위, ~2048까지 128단위 , ~4096 까지 256단위
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256),
		MAX_ALLOC_SIZE = 4096
	};

public:
	MemoryManager();
	~MemoryManager();


	void* Allocate(int32 size);
	void Release(void* ptr);


private:
	vector<MemoryPool*> _pools;

	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1];

};

