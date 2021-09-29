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



void ClientPacketHandler::Handle_S_TEST(BYTE* buffer, int32 len)
{

	BufferReader br(buffer, len);

	if (len < sizeof(PKT_S_TEST))
		return;

	PKT_S_TEST pkt;
	br >> pkt; // 데이터 보낼때 해당 구조체의 변수위치를 보고 보낸다.
	//잘 보면 패킷 구조체의 데이터들과 패킷 데이터를 보냈을때 순서가 일치하다.

	if (pkt.Validate() == false)
		return;

	vector<PKT_S_TEST::BuffsListItem> buffs; // 가변데이터를 담을 배열

	/*KT_S_TEST에 만약 이 배열을 둘경우 가변데이터이기 때문에 구조체의 size를 가늠하기 어렵다.
	 그래서 구조체에는 정적 변수들만 들고 있게 만들고 거기서 동적 변수들을 담기위한
	 위치(offset), 크기(size)를 가져와 동적 변수들을 저장한다.
	 이 위치와 크기 변수들은 가변 데이터의 갯수만큼 만들어야한다. */

	/* 굳이 PKT_S_TEST에 정적 변수들로만 채운 이유는, 구조체 한번에 데이터들을 담을 수있는 편리함과,
	온라인 게임들은 해킹을 대비해서 전달받는 패킷의 사이즈를 잘 검사해서 해킹에 대해 조심해야하기때문. */

	buffs.resize(pkt.buffsCount);
	for (int32 i = 0; i < pkt.buffsCount; i++)
	{
		br >> buffs[i];
	}

	cout << "BufCount : " << pkt.buffsCount << endl;
	for (int32 i = 0; i < pkt.buffsCount; i++)
	{
		cout << "BufInfo : " << buffs[i].buffId << "  " << buffs[i].remainTime << endl;
	}

	// 아직 클라이언트 단에서 조작할수 있는 해킹 이슈에대한 코드는 작성 X

}
