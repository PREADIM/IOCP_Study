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
	int32 DataSize() { return _writePos - _readPos; } // �������ִ� ������ ������
	int32 FreeSize() { return _capacity - _writePos; } // �����͸� �����ִ� ���� ������ ������


private:
	int32 _capacity = 0; // ������ �� ũ��
	int32 _bufferSize = 0; // ���� ������ �ѹ��������ִ� �ִ�ũ�� (����)

	int32 _readPos = 0; // readPos�� �о���ϼ��ִ� �������� ù ���ۺκ��̴�.
	int32 _writePos = 0; // writePos�� �ᱹ Recv�Ҽ��մ� �����Ͱ� �������� �����صδ� Ŀ���̴�. �����͸� Write�ϴ� ������ �ƴϴ�.
	// �׷��鼭�� ���� Recv�� ������ �����͸� ������ ��𼭺��� �����ؾ����� �˷��ִ� ���������̴�.
	Vector<BYTE> _buffer;


};

