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
		// [ PKT_S_TEST ]
		PKT_S_TEST_WRITE pktWriter(1001, 100, 10);


		// [ PKT_S_TEST ][BuffsListItem BuffsListItem BuffsListItem]
		PKT_S_TEST_WRITE::BuffsList buffList = pktWriter.ReserveBuffsList(3);
		buffList[0] = { 100, 1.5f };
		buffList[1] = { 200, 2.3f };
		buffList[2] = { 300, 0.7f };
		
		PKT_S_TEST_WRITE::BuffsVictimsList vic0 = pktWriter.ReserveBuffsVictimsList(&buffList[0], 3); 
		//인자로 그냥 buffList를 하면 안되고, 반드시 &buffList[0]을 해야한다.
		//왜냐면 결국 PacketList안에 있는 컨테이너타입은 결국엔 BuffListItem 이기때문에, 
		//[0]번째링크를 보내주어야 인자로 받는 BuffListItem*이 일치한다.
		//[0]번째 링크를 보내주어도 어처피 PKT_S_TEST_WRITE 클래스 안에있는 BufferWriter에서 알아서 Reserve하면서 알아서 다음 커서로 메모리를 할당해준다.

		{
			vic0[0] = 1000;
			vic0[1] = 2000;
			vic0[2] = 3000;
		}

		PKT_S_TEST_WRITE::BuffsVictimsList vic1 = pktWriter.ReserveBuffsVictimsList(&buffList[1], 1);

		{
			vic1[0] = 1000;
		}

		PKT_S_TEST_WRITE::BuffsVictimsList vic2 = pktWriter.ReserveBuffsVictimsList(&buffList[2], 2);

		{
			vic2[0] = 3000;
			vic2[1] = 5000;
		}


		SendBufferRef sendBuffer = pktWriter.CloseAndReturn();

		GSessionManager.Broadcast(sendBuffer); // 모든애들한테 전달. 브로드 캐스팅

		this_thread::sleep_for(250ms); // 0.25초
	}


	GThreadManager->Join();
}

