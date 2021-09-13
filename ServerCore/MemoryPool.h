#pragma once

enum
{
	SLIST_ALIGNMENT = 16 //16����Ʈ ������ ���� ���
};


//�� <MemoryHeader> ��

DECLSPEC_ALIGN(SLIST_ALIGNMENT) // 16���� ����
struct MemoryHeader : public SLIST_ENTRY // �޸�Ǯ�� �̿��� ��尰�� ���� + ����� �̿��Ͽ� ������ ���� �� ��ü ����ü.
{
	MemoryHeader(int32 size) : _allocSize(size) { }

	static void* AttachHeader(MemoryHeader* header, int32 size) 
	{
		new(header)MemoryHeader(size);
		return reinterpret_cast<void*>(++header);
	}// ��� + �����͸� ���� ����� ���ڷ� ������ ����� allocsize�� �ʱ�ȭ ���ְ� data�� �������ִ� �ּҰ����� �̵��� ��ȯ.

	static MemoryHeader* DetachHeader(void* ptr)
	{
		MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr) - 1;
		//-1�� �ϸ� �� �ּڰ���ŭ -1�� �ȴ�.
		return header;
	}

	int32 _allocSize;
};



//�� <MemoryPool> ��

DECLSPEC_ALIGN(SLIST_ALIGNMENT)
class MemoryPool
{
public:
	MemoryPool(int32 size);
	~MemoryPool();

	void MReturn(MemoryHeader* ptr); // �޸� Ǯ�� �ݳ��ϴ� �Լ�.
	MemoryHeader* MRent(); // �޸� Ǯ���� ������ �Լ�.

private:
	SLIST_HEADER _SLIST_Header; // �߿�
	int32 _allocSize = 0;
	Atomic<int32> _useCount = 0;
	Atomic<int32> _reserveCount = 0;
};

