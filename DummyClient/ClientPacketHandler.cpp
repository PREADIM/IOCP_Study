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
	br >> pkt; // ������ ������ �ش� ����ü�� ������ġ�� ���� ������.
	//�� ���� ��Ŷ ����ü�� �����͵�� ��Ŷ �����͸� �������� ������ ��ġ�ϴ�.

	if (pkt.Validate() == false)
		return;

	vector<PKT_S_TEST::BuffsListItem> buffs; // ���������͸� ���� �迭

	/*KT_S_TEST�� ���� �� �迭�� �Ѱ�� �����������̱� ������ ����ü�� size�� �����ϱ� ��ƴ�.
	 �׷��� ����ü���� ���� �����鸸 ��� �ְ� ����� �ű⼭ ���� �������� �������
	 ��ġ(offset), ũ��(size)�� ������ ���� �������� �����Ѵ�.
	 �� ��ġ�� ũ�� �������� ���� �������� ������ŭ �������Ѵ�. */

	/* ���� PKT_S_TEST�� ���� ������θ� ä�� ������, ����ü �ѹ��� �����͵��� ���� ���ִ� ���԰�,
	�¶��� ���ӵ��� ��ŷ�� ����ؼ� ���޹޴� ��Ŷ�� ����� �� �˻��ؼ� ��ŷ�� ���� �����ؾ��ϱ⶧��. */

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

	// ���� Ŭ���̾�Ʈ �ܿ��� �����Ҽ� �ִ� ��ŷ �̽������� �ڵ�� �ۼ� X

}
