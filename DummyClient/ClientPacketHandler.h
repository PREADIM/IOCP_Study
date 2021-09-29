#pragma once


enum
{
	S_TEST = 1
};


#pragma pack(1) // 1����Ʈ�� ���ڴ�.
struct PKT_S_TEST
{
	struct BuffsListItem
	{
		uint64 buffId;
		float remainTime;
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

