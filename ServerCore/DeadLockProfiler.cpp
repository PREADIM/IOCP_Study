#include "pch.h"
#include "DeadLockProfiler.h"

void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard gaurd(_lock);

	int32 lockId = 0;

	auto findIt = _nameToId.find(name);
	if (findIt == _nameToId.end()) //���ٸ�
	{
		lockId = static_cast<int32>(_nameToId.size());
		_nameToId[name] = lockId;
		_idToName[lockId] = name;
	}
	else
	{
		lockId = findIt->second;
	}


	if (TL_LockStack.empty() == false) // ��������ʴٸ�
	{
		const int32 prevId = TL_LockStack.top(); // ���� �ֱٰ� ������.
		if (lockId != prevId) //���� lockId�� ���� �ֱٿ� ����� Id�� �ٸ����.
		{
			set<int32>& history = _lockHistory[prevId];
			if (history.find(lockId) == history.end()) //������
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
		CRASH("INVALID_UNLOCK");	//���� �ֱٿ� ���� Id�� �ƴҰ�� ũ����

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
	if (_discoveredOrder[now] != -1) // -1�� �ƴϸ� �湮�� ����� �ִ�.
		return;

	// _discoveredOrder[now]�� -1�� ���. �ѹ��� �湮�� �������� ���.
	_discoveredOrder[now] = _discoveredCount++; //�ش� ī��Ʈ ����

	auto findId = _lockHistory.find(now); //[now]�� ����⸦ Ȯ��. �Ѹ���� now������ ���� ���� üũ
	if (findId == _lockHistory.end()) //now�� ������ lock�� ���ٸ�,
	{
		_finished[now] = true; // �湮 ��
		return;
	}

	set<int32>& nextlock = findId->second;
	for (auto nextLockId : nextlock)
	{
		if (_discoveredOrder[nextLockId] == -1) // -1�̶�� ���� �ѹ��� �湮���� �ʾұ⶧���� ������� �ƴϴ�.
		{
			_parent[nextLockId] = now; // [nextLock]�� �θ�� [now] ��� ��.
			DFS(nextLockId);
			continue;
		}
		
		if (_discoveredOrder[nextLockId] > _discoveredOrder[now])
			continue; // [nextLock]�� _discoveredCount�� [now]���� ũ�ٸ�?
					  // ������ �����̴�. (������ ����)

		if (_finished[nextLockId] == false) // ������ ������ �ƴϰ� ������ _finished�� false���
		{
			printf("%s -> %s\n", _idToName[now], _idToName[nextLockId]);

			int32 Id = now; // _parent[now]�� �θ� lockId�� �޾Ƽ� nextLock�� ��ġ�ϴ��� ã�Ƴ���.
			while (true)
			{
				printf("%s -> %s\n", _idToName[_parent[Id]], _idToName[Id]);
				Id = _parent[Id];
				if (Id == nextLockId)
					break; // ã��.
			}

			CRASH("DEADLOCK_DETECTED"); //ũ����
		}

	}


	_finished[now] = true; // �ƹ��ϵ� ������ ���̴�.

}
