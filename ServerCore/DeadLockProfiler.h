#pragma once
#include <stack>
#include <map>
#include <vector>


class DeadLockProfiler
{
public:
	void PushLock(const char* name);
	void PopLock(const char* name);
	void CheckCycle();

private:
	void DFS(int32 now);

private:
	unordered_map<const char*, int32> _nameToId;
	unordered_map<int32, const char*> _idToName;
	map<int32, set<int32>> _lockHistory;

	Mutex _lock;

private:
	vector<int32> _discoveredOrder; //��尡 �߰ߵ� ������ ����ϴ� �迭. �߰ߵ� ����
	int32 _discoveredCount = 0; //��尡 �߰ߵ� ����. �߰��� ī��Ʈ
	vector<bool> _finished; //Dfs(i)�� ����Ǿ��°�? ����
	vector<int32> _parent;
};

