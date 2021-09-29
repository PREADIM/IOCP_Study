#include "GameServerPch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "BufferWriter.h"
#include "ServerPacketHandler.h"



int main()
{
	ServerServiceRef service = MakeShared<ServerService>(
		NetAddress(L"127.0.0.1", 7777),
		MakeShared<IocpCore>(),
		MakeShared<GameSession>,
		100);

	ASSERT_CRASH(service->Start());


	for (int i = 0; i < 5; i++)
	{
		GThreadManager->Launch([=]()
			{
				while (true)
				{
					service->GetIocpCore()->Dispatch();
				}
			});
	}

	char sendData[] = "Hello World";

	while (true)
	{
		vector<BuffsListItem> buffs{ BuffsListItem { 100, 1.5f }, BuffsListItem { 200, 2.3f }, BuffsListItem { 300, 0.7f } };
		SendBufferRef sendBuffer = ServerPacketHandler::Make_S_TEST(1001, 100, 10, buffs);

		GSessionManager.Broadcast(sendBuffer); // 모든애들한테 전달. 브로드 캐스팅

		this_thread::sleep_for(250ms); // 0.25초
	}


	GThreadManager->Join();
}

