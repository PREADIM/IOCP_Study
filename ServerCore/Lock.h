#pragma once
class Lock
{
	enum : uint32
	{
		TIMEOUT_TICK = 10000,
		MAX_SPIN_COUNT = 5000,
		WRITE_THREAD_MASK = 0xFFFF'0000,
		READ_THREAD_MASK = 0x0000'FFFF,
		EMPTY_FLAG = 0x0000'0000
	};

public :
	void WriteLock();
	void WriteUnLock();
	void ReadLock();
	void ReadUnLock();


private :
	Atomic<uint32> _lockFlag = EMPTY_FLAG;
	uint16 _writeCount;
	
};

