#include "GameServerPch.h"
#include "ServerPacketHandler.h"
#include "BufferReader.h"
#include "BufferWriter.h"

void ServerPacketHandler::HandlePacket(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br.Peek(&header);

	switch (header.id)
	{
	default:
		break;
	}

}

SendBufferRef ServerPacketHandler::Make_S_TEST(uint64 id, uint32 hp, uint16 attack, vector<BuffsListItem> buffs)
{

	SendBufferRef sendBuffer = GSendBufferManager->Open(4096); // Open 할때 sendBuffer생성자에 필요한 owner, buffer, allocSize를 다 초기화 해줌.

	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>(); // 헤더를 저장할 곳 미리 마련.

	//id , 체 , 공
	bw << id << hp << attack;

	struct ListHeader
	{
		uint16 offset;
		uint16 count;
	};


	//가변데이터
	ListHeader* buffsHeader = bw.Reserve<ListHeader>();

	buffsHeader->offset = bw.WriteSize(); // 가변데이터가 시작할 커서위치
	buffsHeader->count = buffs.size(); // 인자로 받은 가변데이터의 크기


	for (BuffsListItem& buff : buffs)
	{
		bw << buff.buffId << buff.remainTime;
	}


	header->size = bw.WriteSize(); // 헤더 저장공간을 미리 확보해두었으니 언제든지 접근가능.
	header->id = S_TEST; // 1: text msg

	sendBuffer->Close(bw.WriteSize()); // 다 사용하면 닫아주기.

	return sendBuffer;
}
