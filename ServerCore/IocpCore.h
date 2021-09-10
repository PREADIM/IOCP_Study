#pragma once

// �� IocpObject ��
// 
//�� Ŭ������ ����ؼ� Iocp���� �̿��� ������Ʈ���� �����.
class IocpObject : public enable_shared_from_this<IocpObject>
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvnet, int32 numOfByte = 0) abstract;
};





// �� IocpCore ��

class IocpCore // Iocp �ڵ��� �ʱ�ȭ�ϰ� ���� �����ϰ� GetQueuedCompletionStatus�� ������Iocp�� ã�Ƽ� �����Ѵ�.
{
public:
	IocpCore();
	~IocpCore();

	HANDLE GetHandle() { return _iocpHandle; }

	bool Register(IocpObjectRef iocpObject); // iocpObject�ȿ� �ִ� HANDLE�� iocpHandle�� ����
	bool Dispatch(uint32 timeOutMs = INFINITE);


private:
	HANDLE _iocpHandle;

};

