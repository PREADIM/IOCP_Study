#pragma once




class RecvBuffer
{

	enum { BUFFER_COUNT = 10 };

public:
	RecvBuffer(int32 bufferSize);
	~RecvBuffer();


	void Clean();
	bool OnRead(int32 numOfBytes);
	bool OnWrite(int32 numOfBytes);


	BYTE* ReadPos() { return &_buffer[_readPos]; }
	BYTE* WritePos() { return &_buffer[_writePos]; }
	int32 DataSize() { return _writePos - _readPos; } // 읽을수있는 데이터 사이즈
	int32 FreeSize() { return _capacity - _writePos; } // 데이터를 쓸수있는 남은 데이터 사이즈


private:
	int32 _capacity = 0; // 버퍼의 총 크기
	int32 _bufferSize = 0; // 단일 버퍼의 한번에쓸수있는 최대크기 (단위)

	int32 _readPos = 0; // readPos는 읽어들일수있는 데이터의 첫 시작부분이다.
	int32 _writePos = 0; // writePos는 결국 Recv할수잇는 데이터가 들어왔을때 저장해두는 커서이다. 데이터를 Write하는 느낌이 아니다.
	// 그러면서도 지금 Recv를 했을때 데이터를 버퍼의 어디서부터 저장해야할지 알려주는 시작지점이다.
	Vector<BYTE> _buffer;


};

