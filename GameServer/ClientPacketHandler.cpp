#include "GameServerPch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"
#include "BufferWriter.h"


PacketHandlerFunc GPacketHandler[UINT16_MAX];


bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	//TODO : log Ȯ��

	return true;
}

bool Handle_C_TEST(PacketSessionRef& session, Protocol::C_TEST& pkt)
{
	return false;
}

bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
{
	return false;
}


bool Handle_C_TEST(PacketSessionRef& session, Protocol::S_TEST& pkt)
{
	return true;
}





/*void ServerPacketHandler::HandlePacket(BYTE* buffer, int32 len)
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

	SendBufferRef sendBuffer = GSendBufferManager->Open(4096); // Open �Ҷ� sendBuffer�����ڿ� �ʿ��� owner, buffer, allocSize�� �� �ʱ�ȭ ����.

	BufferWriter bw(sendBuffer->Buffer(), sendBuffer->AllocSize());
	PacketHeader* header = bw.Reserve<PacketHeader>(); // ����� ������ �� �̸� ����.

	//id , ü , ��
	bw << id << hp << attack;

	struct ListHeader
	{
		uint16 offset;
		uint16 count;
	};


	//����������
	ListHeader* buffsHeader = bw.Reserve<ListHeader>();

	buffsHeader->offset = bw.WriteSize(); // ���������Ͱ� ������ Ŀ����ġ
	buffsHeader->count = buffs.size(); // ���ڷ� ���� ������������ ũ��


	for (BuffsListItem& buff : buffs)
	{
		bw << buff.buffId << buff.remainTime;
	}


	header->size = bw.WriteSize(); // ��� ��������� �̸� Ȯ���صξ����� �������� ���ٰ���. �������� �� ������
	header->id = S_TEST; // 1: text msg

	sendBuffer->Close(bw.WriteSize()); // �� ����ϸ� �ݾ��ֱ�.

	return sendBuffer;
}*/

