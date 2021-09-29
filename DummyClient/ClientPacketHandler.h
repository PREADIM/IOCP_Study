#pragma once


enum
{
	S_TEST = 1
};


#pragma pack(1) // 1바이트씩 쓰겠다.
struct PKT_S_TEST
{
	struct BuffsListItem
	{
		uint64 buffId;
		float remainTime;
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

		size += sizeof(PKT_S_TEST);
		size += buffsCount * sizeof(BuffsListItem);
		if (size != packetSize)
			return false;

		if (buffsOffset + buffsCount * sizeof(BuffsListItem) > packetSize)
			return false;

		return true;
	}

};
#pragma pack()



class ClientPacketHandler
{
public :
	static void HandlePacket(BYTE* buffer, int32 len);
	static void Handle_S_TEST(BYTE* buffer, int32 len);
};

