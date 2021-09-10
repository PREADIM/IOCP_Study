#pragma once

// ★ IocpObject ★
// 
//이 클래스를 상속해서 Iocp에서 이용할 오브젝트들을 만든다.
class IocpObject : public enable_shared_from_this<IocpObject>
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvnet, int32 numOfByte = 0) abstract;
};





// ★ IocpCore ★

class IocpCore // Iocp 핸들을 초기화하고 값을 설정하고 GetQueuedCompletionStatus로 가동된Iocp를 찾아서 실행한다.
{
public:
	IocpCore();
	~IocpCore();

	HANDLE GetHandle() { return _iocpHandle; }

	bool Register(IocpObjectRef iocpObject); // iocpObject안에 있는 HANDLE에 iocpHandle을 연동
	bool Dispatch(uint32 timeOutMs = INFINITE);


private:
	HANDLE _iocpHandle;

};

