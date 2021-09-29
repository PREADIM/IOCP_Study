#pragma once


class BufferReader
{
public:
	BufferReader();
	BufferReader(BYTE* buffer, uint32 size, uint32 pos = 0);
	~BufferReader();


	template<typename T>
	bool Peek(T* dest) { return Peek(dest, sizeof(T)); } // 더 간단하게 쓰기위한 함수. 이런 방법도있다는것을 참고.
	bool Peek(void* dest, uint32 len); // 살짝 엿보기
	
	template<typename T>
	bool Read(T* dest) { return Read(dest, sizeof(T)); } // 더 간단하게 쓰기위한 함수. 이런 방법도있다는것을 참고.
	bool Read(void* dest, uint32 len); // 데이터를 읽어들이기


	template<typename T>
	BufferReader& operator>>(T& dest);


public:
	BYTE* Buffer() { return _buffer; }
	uint32 Size() { return _size; }
	uint32 ReadSize() { return _pos; }
	uint32 FreeSize() { return _size - _pos; }

private:
	BYTE* _buffer = nullptr;
	uint32 _size = 0;
	uint32 _pos = 0;


};

template<typename T>
inline BufferReader& BufferReader::operator>>(T& dest)
{
	dest = *reinterpret_cast<T*>(&_buffer[_pos]);
	_pos += sizeof(T);
	return *this;
}