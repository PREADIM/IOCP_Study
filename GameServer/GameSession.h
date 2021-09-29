#pragma once
#include "Session.h"

class GameSession : public PacketSession
	// �������� �����ϴ� �� GameSession�� Ŭ���̾�Ʈ�� Session�� �����͸� �ְ�ް� �ϱ����� ������ ��ǥ���̴�.
{
public:

	~GameSession()
	{
		cout << "~GameSession" << endl;
	}

	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;
	
};
