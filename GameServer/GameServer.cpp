#include "GameServerPch.h"
#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "GameSession.h"
#include "GameSessionManager.h"
#include "BufferWriter.h"
#include "ClientPacketHandler.h"
#include "Protocol.pb.h"



int main()
{
	ClientPacketHandler::Init();

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
		Protocol::S_TEST pkt;
		pkt.set_id(1000);
		pkt.set_hp(100);
		pkt.set_attack(10);


		//auto m = pkt.mutable_buffs();
		//m->Add();


		{
			Protocol::BuffData* data = pkt.add_buffs(); // 해당 데이터 자리로 가서 인자 저장. (Reserve로 받아서 저장하는 느낌)
			data->set_buffid(100);
			data->set_remaintime(1.2f);
			data->add_victims(4000);
		}

		{
			Protocol::BuffData* data = pkt.add_buffs(); // 해당 데이터 자리로 가서 인자 저장. (Reserve로 받아서 저장하는 느낌)
			data->set_buffid(200);
			data->set_remaintime(2.5f);
			data->add_victims(1000);
			data->add_victims(2000);
		}

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(pkt);

		GSessionManager.Broadcast(sendBuffer); // 모든애들한테 전달. 브로드 캐스팅

		this_thread::sleep_for(250ms); // 0.25초
	}


	GThreadManager->Join();
}

