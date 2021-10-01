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




template<typename T, typename C> // T는 리턴을 하기위해서 C는 컨테이너 그자체
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


#pragma pack(1) // 1바이트씩 쓰겠다.
struct PKT_S_TEST
{
	struct BuffsListItem
	{
		uint64 buffId;
		float remainTime;

		uint16 victimsOffset;
		uint16 victimsCount;
	};

	//PacketHeader에 있던 것들
	uint16 packetSize;
	uint16 packetId;

	uint64 id;
	uint32 hp;
	uint16 attack;

	uint16 buffsOffset; //가변데이터가 시작하는 커서를 가리킴
	uint16 buffsCount;

	//vector<BuffsListItem> buffs; 
	// 이건 없어도 된다. 왜냐하면 offset과 Count를 알고있으므로 그걸 이용해서 아예 새로운 배열에 받아도 되기때문.
	// 패킷 구조체의 용도는 패킷 데이터들을 담을 수도 있지만 패킷을 가져오기 위한 수단에 불과하다.


	bool Validate()
	{
		uint32 size = 0;

		size += sizeof(PKT_S_TEST); // 정적 변수들 총 크기
		if (packetSize < size) // 가변데이터의 크기가 아직 안 더해졌기때문에 반드시 packetSize가 작아야됨.
			return false;


		size += buffsCount * sizeof(BuffsListItem); // 가변 데이터크기까지 더하기.
		if (size != packetSize) //size는 결국 총 패킷의 사이즈를 가리키는 packetSize와 같기때문에 같아야함.
			return false;


		if (buffsOffset + buffsCount * sizeof(BuffsListItem) > packetSize)
			return false;

		return true;
	}


	using BuffsList = PacketList<PKT_S_TEST::BuffsListItem>; // typedef 같은 느낌

	BuffsList GetBuffsList()
	{
		BYTE* data = reinterpret_cast<BYTE*>(this);
		data += buffsOffset; // 가변 데이터가 있는 곳으로 이동
		return BuffsList(reinterpret_cast<PKT_S_TEST::BuffsListItem*>(data), buffsCount);
	}


};




// -------------------------------------------------------------------------------------------




class PKT_S_TEST_WRITE
{
public:
	using BuffsListItem = PKT_S_TEST::BuffsListItem;
	using BuffsList = PacketList<PKT_S_TEST::BuffsListItem>; // typedef 같은 느낌
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


