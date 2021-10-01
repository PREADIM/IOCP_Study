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


#pragma pack(1) // 1����Ʈ�� ���ڴ�.
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

		if (buffsOffset + buffsCount * sizeof(BuffsListItem) > packetSize)
			return false;

		size += buffsCount * sizeof(BuffsListItem); // ���� ������ũ����� ���ϱ�.
		
		
		BuffsList buffList = GetBuffsList();
		for (int32 i = 0; i < buffList.Count(); i++)
		{
			if (buffList[i].Validate((BYTE*)this, packetSize, size) == false)
			return false;
		}
		
		
		//���� ũ�� ��
		if (size != packetSize) //size�� �ᱹ �� ��Ŷ�� ����� ����Ű�� packetSize�� ���⶧���� ���ƾ���.
			return false;



		return true;
	}


	using BuffsList = PacketList<PKT_S_TEST::BuffsListItem>; // typedef ���� ����
	using BuffsVictimsList = PacketList<uint64>;

	BuffsList GetBuffsList() // ���� ���
	{
		BYTE* data = reinterpret_cast<BYTE*>(this);
		data += buffsOffset; // ���� �����Ͱ� �ִ� ������ �̵�
		return BuffsList(reinterpret_cast<PKT_S_TEST::BuffsListItem*>(data), buffsCount);
	}


	BuffsVictimsList GetBuffVictimList(BuffsListItem* buffsItem) //�ش� ���������Ϳ� ���ع��� ����� ���� ��� / [���������� �ȿ� ����������]
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
	// buffer ��ü�� ���ʿ� PKT_S_TEST�̱⶧���� ��������.

	/*PKT_S_TEST pkt;
	br >> pkt; // ������ ������ �ش� ����ü�� ������ġ�� ���� ������.
	//�� ���� ��Ŷ ����ü�� �����͵�� ��Ŷ �����͸� �������� ������ ��ġ�ϴ�.*/

	if (pkt->Validate() == false)
		return;

	//vector<PKT_S_TEST::BuffsListItem> buffs; // ���������͸� ���� �迭

	/*KT_S_TEST�� ���� �� �迭�� �Ѱ�� �����������̱� ������ ����ü�� size�� �����ϱ� ��ƴ�.
	 �׷��� ����ü���� ���� �����鸸 ��� �ְ� ����� �ű⼭ ���� �������� �������
	 ��ġ(offset), ũ��(size)�� ������ ���� �������� �����Ѵ�.
	 �� ��ġ�� ũ�� �������� ���� �������� ������ŭ �������Ѵ�. */

	/* ���� PKT_S_TEST�� ���� ������θ� ä�� ������, ����ü �ѹ��� �����͵��� ���� ���ִ� ���԰�,
	�¶��� ���ӵ��� ��ŷ�� ����ؼ� ���޹޴� ��Ŷ�� ����� �� �˻��ؼ� ��ŷ�� ���� �����ؾ��ϱ⶧��. */

	
	PKT_S_TEST::BuffsList buffs = pkt->GetBuffsList(); // ���������� ����.


	/*cout << "BufCount : " << buffs.Count() << endl;
	for (int32 i = 0; i < buffs.Count(); i++)
	{
		cout << "BufInfo : " << buffs[i].buffId << "  " << buffs[i].remainTime << endl;
	}

	// ���� Ŭ���̾�Ʈ �ܿ��� �����Ҽ� �ִ� ��ŷ �̽������� �ڵ�� �ۼ� X


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
