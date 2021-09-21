#pragma once
#include "Session.h"

class GameSession : public Session
	// 서버에서 생성하는 이 GameSession은 클라이언트의 Session에 데이터를 주고받고 하기위한 일종의 매표소이다.
{
public:

	~GameSession()
	{
		cout << "~GameSession" << endl;
	}

	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual int32 OnRecv(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;
	
};
