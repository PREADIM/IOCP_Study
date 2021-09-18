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
	void WriteLock(const char* name);
	void WriteUnLock(const char* name);
	void ReadLock(const char* name);
	void ReadUnLock(const char* name);


private :
	Atomic<uint32> _lockFlag = EMPTY_FLAG;
	uint16 _writeCount = 0;
	
};




class ReadLockGuard : public Lock
{
public:
	ReadLockGuard(Lock& lock, const char* name) : _lock(lock), _name(name) { _lock.ReadLock(name); }
	~ReadLockGuard() { _lock.ReadUnLock(_name); }

private:
	Lock& _lock;
	const char* _name;
};



class WriteLockGuard : public Lock
{
public :
	WriteLockGuard(Lock& lock, const char* name) : _lock(lock), _name(name) { _lock.WriteLock(name); }
	~WriteLockGuard() { _lock.WriteUnLock(_name); }

private :
	Lock& _lock;
	const char* _name;
};

