#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"

IocpCore::IocpCore()
{
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	ASSERT_CRASH(_iocpHandle != INVALID_HANDLE_VALUE);

}

IocpCore::~IocpCore()
{
	::CloseHandle(_iocpHandle);

}

bool IocpCore::Register(IocpObjectRef iocpObject)
{
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, 0, 0); // 핸들에 iocp핸들 대입.
}

bool IocpCore::Dispatch(uint32 timeOutMs)
{
	DWORD numOfByte = 0; // 데이터의 크기
	ULONG_PTR key = 0;
	IocpEvent* iocpEvent = nullptr; // 데이터를 받아와서 이걸로 무엇을 행동할지 판단한다.

	if (::GetQueuedCompletionStatus(_iocpHandle, &numOfByte, &key, reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeOutMs))
	{
		// true -> _iocpHandle에 무언가 낌새가 왔다는 뜻.
		IocpObjectRef iocpObject = iocpEvent->owner; 
		// GetQueuedCompletionStatus는 LPOVERLAPPED형 데이터를 받아올수있기때문에 이걸로 판단.
		iocpObject->Dispatch(iocpEvent, numOfByte);
	}
	else
	{
		//false 아직 _iocpHandle에 낌새가 안왔다는 뜻
		int32 errCode = ::WSAGetLastError(); // 마지막 에러
		switch (errCode)
		{
		case WAIT_TIMEOUT :
			return false; // 시간제한 끝.
		default :
			// 로그 찍기
			IocpObjectRef iocpObject = iocpEvent->owner;
			iocpObject->Dispatch(iocpEvent, numOfByte);
		}
	}

	return true;
}
