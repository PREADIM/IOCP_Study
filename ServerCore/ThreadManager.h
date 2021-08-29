#pragma once

#include <thread>
#include <functional>

class ThreadManager
{

public :
	ThreadManager();
	~ThreadManager();

	void InitTLS();
	void DestoryTLS();
	
	void Launch(function<void(void)> callback);
	void Join();


private :
	Mutex mutex;
	vector<thread> thread_vector;
};

