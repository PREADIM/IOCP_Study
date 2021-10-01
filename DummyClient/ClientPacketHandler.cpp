#include "DummyClientPch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"



void ClientPacketHandler::HandlePacket(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br >> header;

	switch (header.id)
	{
	case S_TEST:
		Handle_S_TEST(buffer, len);
		break;
	}


}


#pragma pack(1) // 1바이트씩 쓰겠다.
struct PKT_S_TEST
{
	struct BuffsListItem
	{
		uint64 buffId;
		float remainTime;

		uint16 victimsOffset;
		uint16 victimsCount;


		bool Validate(BYTE* packetStart, uint16 packetSize, uint32& size)
		{
			if (victimsOffset + victimsCount * sizeof(uint64) > packetSize)
				return false;
			
			size += victimsCount * sizeof(uint64);
			return true;
		}

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

		if (buffsOffset + buffsCount * sizeof(BuffsListItem) > packetSize)
			return false;

		size += buffsCount * sizeof(BuffsListItem); // 가변 데이터크기까지 더하기.
		
		
		BuffsList buffList = GetBuffsList();
		for (int32 i = 0; i < buffList.Count(); i++)
		{
			if (buffList[i].Validate((BYTE*)this, packetSize, size) == false)
			return false;
		}
		
		
		//최종 크기 비교
		if (size != packetSize) //size는 결국 총 패킷의 사이즈를 가리키는 packetSize와 같기때문에 같아야함.
			return false;



		return true;
	}


	using BuffsList = PacketList<PKT_S_TEST::BuffsListItem>; // typedef 같은 느낌
	using BuffsVictimsList = PacketList<uint64>;

	BuffsList GetBuffsList() // 버프 출력
	{
		BYTE* data = reinterpret_cast<BYTE*>(this);
		data += buffsOffset; // 가변 데이터가 있는 곳으로 이동
		return BuffsList(reinterpret_cast<PKT_S_TEST::BuffsListItem*>(data), buffsCount);
	}


	BuffsVictimsList GetBuffVictimList(BuffsListItem* buffsItem) //해당 버프데이터에 피해받은 사람들 정보 출력 / [가변데이터 안에 가변데이터]
	{
		BYTE* data = reinterpret_cast<BYTE*>(this);
		data += buffsItem->victimsOffset;
		return BuffsVictimsList(reinterpret_cast<uint64*>(data), buffsItem->victimsCount);
	}

};

#pragma pack()



void ClientPacketHandler::Handle_S_TEST(BYTE* buffer, int32 len)
{

	BufferReader br(buffer, len);

	if (len < sizeof(PKT_S_TEST))
		return;

	PKT_S_TEST* pkt = reinterpret_cast<PKT_S_TEST*>(buffer);
	// buffer 자체가 애초에 PKT_S_TEST이기때문에 문제없음.

	/*PKT_S_TEST pkt;
	br >> pkt; // 데이터 보낼때 해당 구조체의 변수위치를 보고 보낸다.
	//잘 보면 패킷 구조체의 데이터들과 패킷 데이터를 보냈을때 순서가 일치하다.*/

	if (pkt->Validate() == false)
		return;

	//vector<PKT_S_TEST::BuffsListItem> buffs; // 가변데이터를 담을 배열

	/*KT_S_TEST에 만약 이 배열을 둘경우 가변데이터이기 때문에 구조체의 size를 가늠하기 어렵다.
	 그래서 구조체에는 정적 변수들만 들고 있게 만들고 거기서 동적 변수들을 담기위한
	 위치(offset), 크기(size)를 가져와 동적 변수들을 저장한다.
	 이 위치와 크기 변수들은 가변 데이터의 갯수만큼 만들어야한다. */

	/* 굳이 PKT_S_TEST에 정적 변수들로만 채운 이유는, 구조체 한번에 데이터들을 담을 수있는 편리함과,
	온라인 게임들은 해킹을 대비해서 전달받는 패킷의 사이즈를 잘 검사해서 해킹에 대해 조심해야하기때문. */

	
	PKT_S_TEST::BuffsList buffs = pkt->GetBuffsList(); // 가변데이터 추출.


	/*cout << "BufCount : " << buffs.Count() << endl;
	for (int32 i = 0; i < buffs.Count(); i++)
	{
		cout << "BufInfo : " << buffs[i].buffId << "  " << buffs[i].remainTime << endl;
	}

	// 아직 클라이언트 단에서 조작할수 있는 해킹 이슈에대한 코드는 작성 X


	for (auto it = buffs.begin(); it != buffs.end(); ++it)
	{
		cout << "BufInfo : " << it->buffId << " " << it->remainTime << endl;
	}*/

	for (auto& buff : buffs)
	{
		cout << "BufInfo : " << buff.buffId << " " << buff.remainTime << endl;


		PKT_S_TEST::BuffsVictimsList victims = pkt->GetBuffVictimList(&buff);

		cout << "Victims Count :" << victims.Count() << endl;

		for (auto& victim : victims)
		{
			cout << "Victim : " << victim << endl;
		}

	}

}
