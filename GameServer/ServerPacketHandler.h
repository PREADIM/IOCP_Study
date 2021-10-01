#pragma once
#include "BufferReader.h"
#include "BufferWriter.h"


enum
{
	S_TEST = 1
};


struct BuffsListItem
{
	uint64 buffId;
	float remainTime;
};



// -------------------------------------------------------------------------------------




template<typename T, typename C> // T�� ������ �ϱ����ؼ� C�� �����̳� ����ü
class PacketIterator
{
public:
	PacketIterator(C& container, uint16 index) : _container(container), _index(index) { }

	bool				operator!=(const PacketIterator& other) { return _index != other._index; }

	const T& operator*() const { return _container[_index]; }

	T& operator*() { return _container[_index]; }

	T* operator->() const { return &_container[_index]; }


	PacketIterator& operator++()
	{
		_index++;
		return *this;
	}

	PacketIterator		operator++(int32)
	{
		PacketIterator ret = *this;
		++_index;
		return ret;
	}



private:
	C& _container; // PacketLsit<PKT_S_TEST::BuffsListItem>&
	uint16 _index;
};




template<typename T>
class PacketList
{
public:
	PacketList() : _data(nullptr), _count(0) {}
	PacketList(T* data, uint16 count) : _data(data), _count(count) {}

	T& operator[](uint16 index)
	{
		ASSERT_CRASH(index < _count);
		return _data[index];
	}

	uint16 Count() { return _count; }

	PacketIterator<T, PacketList<T>> begin() { return PacketIterator<T, PacketList<T>>(*this, 0); }
	PacketIterator<T, PacketList<T>> end() { return PacketIterator<T, PacketList<T>>(*this, _count); }


private:
	T* _data;
	uint16 _count;
};


// -----------------------------------------------------------------------




class ServerPacketHandler
{
public:
	static void HandlePacket(BYTE* buffer, int32 len);


};



// -------------------------------------------------------------------------


#pragma pack(1) // 1����Ʈ�� ���ڴ�.
struct PKT_S_TEST
{
	struct BuffsListItem
	{
		uint64 buffId;
		float remainTime;

		uint16 victimsOffset;
		uint16 victimsCount;
	};

	//PacketHeader�� �ִ� �͵�
	uint16 packetSize;
	uint16 packetId;

	uint64 id;
	uint32 hp;
	uint16 attack;

	uint16 buffsOffset; //���������Ͱ� �����ϴ� Ŀ���� ����Ŵ
	uint16 buffsCount;

	//vector<BuffsListItem> buffs; 
	// �̰� ��� �ȴ�. �ֳ��ϸ� offset�� Count�� �˰������Ƿ� �װ� �̿��ؼ� �ƿ� ���ο� �迭�� �޾Ƶ� �Ǳ⶧��.
	// ��Ŷ ����ü�� �뵵�� ��Ŷ �����͵��� ���� ���� ������ ��Ŷ�� �������� ���� ���ܿ� �Ұ��ϴ�.


	bool Validate()
	{
		uint32 size = 0;

		size += sizeof(PKT_S_TEST); // ���� ������ �� ũ��
		if (packetSize < size) // ������������ ũ�Ⱑ ���� �� �������⶧���� �ݵ�� packetSize�� �۾ƾߵ�.
			return false;


		size += buffsCount * sizeof(BuffsListItem); // ���� ������ũ����� ���ϱ�.
		if (size != packetSize) //size�� �ᱹ �� ��Ŷ�� ����� ����Ű�� packetSize�� ���⶧���� ���ƾ���.
			return false;


		if (buffsOffset + buffsCount * sizeof(BuffsListItem) > packetSize)
			return false;

		return true;
	}


	using BuffsList = PacketList<PKT_S_TEST::BuffsListItem>; // typedef ���� ����

	BuffsList GetBuffsList()
	{
		BYTE* data = reinterpret_cast<BYTE*>(this);
		data += buffsOffset; // ���� �����Ͱ� �ִ� ������ �̵�
		return BuffsList(reinterpret_cast<PKT_S_TEST::BuffsListItem*>(data), buffsCount);
	}


};




// -------------------------------------------------------------------------------------------




class PKT_S_TEST_WRITE
{
public:
	using BuffsListItem = PKT_S_TEST::BuffsListItem;
	using BuffsList = PacketList<PKT_S_TEST::BuffsListItem>; // typedef ���� ����
	using BuffsVictimsList = PacketList<uint64>;

	PKT_S_TEST_WRITE(uint64 id, uint32 hp, uint16 attack)
	{
		_sendBuffer = GSendBufferManager->Open(4096);
		_bw = BufferWriter(_sendBuffer->Buffer(), _sendBuffer->AllocSize());

		_pkt = _bw.Reserve<PKT_S_TEST>();
		_pkt->packetSize = 0;
		_pkt->packetId = S_TEST;
		_pkt->id = id;
		_pkt->hp = hp;
		_pkt->attack = attack;
		_pkt->buffsOffset = 0;
		_pkt->buffsCount = 0;

	}

	BuffsList ReserveBuffsList(uint16 buffCount)
	{
		BuffsListItem* firstBuffsListItem = _bw.Reserve<BuffsListItem>(buffCount);

		_pkt->buffsOffset = (uint64)firstBuffsListItem - (uint64)_pkt;
		_pkt->buffsCount = buffCount;
		return BuffsList(firstBuffsListItem, buffCount);
	}


	BuffsVictimsList ReserveBuffsVictimsList(BuffsListItem* buffsItem, uint16 victimsCount)
	{
		uint64* firstVictimsListItem = _bw.Reserve<uint64>(victimsCount);
		buffsItem->victimsOffset = (uint64)firstVictimsListItem - (uint64)_pkt;
		buffsItem->victimsCount = victimsCount;
		return BuffsVictimsList(firstVictimsListItem, victimsCount);
	}


	SendBufferRef CloseAndReturn()
	{
		_pkt->packetSize = _bw.WriteSize();

		_sendBuffer->Close(_bw.WriteSize());
		return _sendBuffer;
	}


private:
	PKT_S_TEST* _pkt = nullptr;
	SendBufferRef _sendBuffer;
	BufferWriter _bw;

};



#pragma pack()


