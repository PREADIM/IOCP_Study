#include "pch.h"
#include "DeadLockProfiler.h"

void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard gaurd(_lock);

	int32 lockId = 0;

	auto findIt = _nameToId.find(name);
	if (findIt == _nameToId.end()) //없다면
	{
		lockId = static_cast<int32>(_nameToId.size());
		_nameToId[name] = lockId;
		_idToName[lockId] = name;
	}
	else
	{
		lockId = findIt->second;
	}


	if (TL_LockStack.empty() == false) // 비어있지않다면
	{
		const int32 prevId = TL_LockStack.top(); // 가장 최근것 꺼내옴.
		if (lockId != prevId) //현재 lockId랑 가장 최근에 저장된 Id가 다를경우.
		{
			set<int32>& history = _lockHistory[prevId];
			if (history.find(lockId) == history.end()) //없으면
			{
				history.insert(lockId);
				CheckCycle();
			}
		}
	}

	TL_LockStack.push(lockId);		
}

void DeadLockProfiler::PopLock(const char* name)
{
	LockGuard guard(_lock);

	if (TL_LockStack.empty())
	{
		CRASH("MULTIPLE_UNLOCK");
	}

	int32 lockId = _nameToId[name];
	if (TL_LockStack.top() != lockId)
		CRASH("INVALID_UNLOCK");	//가장 최근에 넣은 Id가 아닐경우 크래시

	TL_LockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	const int32 lockCount = static_cast<int32>(_nameToId.size());
	_discoveredOrder = vector<int32>(lockCount, -1);
	_discoveredCount = 0;
	_finished = vector<bool>(lockCount, false);
	_parent = vector<int32>(lockCount, -1);

	for (int32 i = 0; i < lockCount; i++)
	{
		DFS(i);
	}

	_discoveredOrder.clear();
	_finished.clear();
	_parent.clear();

}

void DeadLockProfiler::DFS(int32 now)
{
	if (_discoveredOrder[now] != -1) // -1이 아니면 방문한 기록이 있다.
		return;

	// _discoveredOrder[now]가 -1인 경우. 한번도 방문을 하지않은 경우.
	_discoveredOrder[now] = _discoveredCount++; //해당 카운트 저장

	auto findId = _lockHistory.find(now); //[now]의 연대기를 확인. 한마디로 now다음에 들어온 락을 체크
	if (findId == _lockHistory.end()) //now의 다음에 lock이 없다면,
	{
		_finished[now] = true; // 방문 끝
		return;
	}

	set<int32>& nextlock = findId->second;
	for (auto nextLockId : nextlock)
	{
		if (_discoveredOrder[nextLockId] == -1) // -1이라는 뜻은 한번도 방문하지 않았기때문에 데드락이 아니다.
		{
			_parent[nextLockId] = now; // [nextLock]의 부모는 [now] 라는 뜻.
			DFS(nextLockId);
			continue;
		}
		
		if (_discoveredOrder[nextLockId] > _discoveredOrder[now])
			continue; // [nextLock]의 _discoveredCount가 [now]보다 크다면?
					  // 지극히 정상이다. (순방향 간선)

		if (_finished[nextLockId] == false) // 순방향 간선도 아니고 심지어 _finished도 false라면
		{
			printf("%s -> %s\n", _idToName[now], _idToName[nextLockId]);

			int32 Id = now; // _parent[now]의 부모 lockId를 받아서 nextLock과 일치하는지 찾아낸다.
			while (true)
			{
				printf("%s -> %s\n", _idToName[_parent[Id]], _idToName[Id]);
				Id = _parent[Id];
				if (Id == nextLockId)
					break; // 찾음.
			}

			CRASH("DEADLOCK_DETECTED"); //크래시
		}

	}


	_finished[now] = true; // 아무일도 없으면 끝이다.

}
