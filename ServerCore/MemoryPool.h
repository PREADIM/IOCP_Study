#pragma once

enum
{
	SLIST_ALIGNMENT = 16 //16바이트 정렬을 위한 상수
};


//★ <MemoryHeader> ★

DECLSPEC_ALIGN(SLIST_ALIGNMENT) // 16진수 정렬
struct MemoryHeader : public SLIST_ENTRY // 메모리풀을 이용할 노드같은 단위 + 헤더를 이용하여 데이터 조립 및 해체 구조체.
{
	MemoryHeader(int32 size) : _allocSize(size) { }

	static void* AttachHeader(MemoryHeader* header, int32 size) 
	{
		new(header)MemoryHeader(size);
		return reinterpret_cast<void*>(++header);
	}// 헤더 + 데이터를 받을 사이즈를 인자로 보내서 헤더에 allocsize를 초기화 해주고 data를 받을수있는 주소값으로 이동후 반환.

	static MemoryHeader* DetachHeader(void* ptr)
	{
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr) - 1;
		//-1을 하면 그 주솟값만큼 -1이 된다.
		return header;
	}

	int32 _allocSize;
};



//★ <MemoryPool> ★

DECLSPEC_ALIGN(SLIST_ALIGNMENT)
class MemoryPool
{
public:
	MemoryPool(int32 size);
	~MemoryPool();

	void MReturn(MemoryHeader* ptr); // 메모리 풀에 반납하는 함수.
	MemoryHeader* MRent(); // 메모리 풀에서 빌리는 함수.

private:
	SLIST_HEADER _SLIST_Header; // 중요
	int32 _allocSize = 0;
	Atomic<int32> _useCount = 0;
	Atomic<int32> _reserveCount = 0;
};

