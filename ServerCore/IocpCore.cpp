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
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, 0, 0); // �ڵ鿡 iocp�ڵ� ����.
}

bool IocpCore::Dispatch(uint32 timeOutMs)
{
	DWORD numOfByte = 0; // �������� ũ��
	ULONG_PTR key = 0;
	IocpEvent* iocpEvent = nullptr; // �����͸� �޾ƿͼ� �̰ɷ� ������ �ൿ���� �Ǵ��Ѵ�.

	if (::GetQueuedCompletionStatus(_iocpHandle, &numOfByte, &key, reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeOutMs))
	{
		// true -> _iocpHandle�� ���� ������ �Դٴ� ��.
		IocpObjectRef iocpObject = iocpEvent->owner; 
		// GetQueuedCompletionStatus�� LPOVERLAPPED�� �����͸� �޾ƿü��ֱ⶧���� �̰ɷ� �Ǵ�.
		iocpObject->Dispatch(iocpEvent, numOfByte);
	}
	else
	{
		//false ���� _iocpHandle�� ������ �ȿԴٴ� ��
		int32 errCode = ::WSAGetLastError(); // ������ ����
		switch (errCode)
		{
		case WAIT_TIMEOUT :
			return false; // �ð����� ��.
		default :
			// �α� ���
			IocpObjectRef iocpObject = iocpEvent->owner;
			iocpObject->Dispatch(iocpEvent, numOfByte);
		}
	}

	return true;
}
