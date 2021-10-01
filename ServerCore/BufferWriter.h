#pragma once


class BufferWriter
{
public:
	BufferWriter();
	BufferWriter(BYTE* buffer, uint32 size, uint32 pos = 0);
	~BufferWriter();


	template<typename T>
	bool Write(T* src) { return Write(src, sizeof(T)); } // 더 간단하게 쓰기위한 함수. 이런 방법도있다는것을 참고.
	bool Write(void* src, uint32 len); // 데이터 작성

	template<typename T>
	T* Reserve(uint16 count = 1);

	/*template<typename T>
	BufferWriter& operator<<(const T& src);*/

	template<typename T>
	BufferWriter& operator<<(T&& src);


public:
	BYTE* Buffer() { return _buffer; }
	uint32 Size() { return _size; }
	uint32 WriteSize() { return _pos; }
	uint32 FreeSize() { return _size - _pos; }

private:
	BYTE* _buffer = nullptr;
	uint32 _size = 0;
	uint32 _pos = 0;


};


template<typename T>
inline T* BufferWriter::Reserve(uint16 count)
{
	if (FreeSize() < sizeof(T) *count)
		return nullptr;
	
	T* ret = reinterpret_cast<T*>(&_buffer[_pos]); // 해당 공간을 주고
	_pos += sizeof(T) * count; // 커서 옮기기 
	return ret;
}



/*inline BufferWriter& BufferWriter::operator<<(const T& src)
{
	*reinterpret_cast<T*>(&_buffer[_pos]) = src;
	_pos += sizeof(T);
	return *this;
}*/

template<typename T>
inline BufferWriter& BufferWriter::operator<<(T&& src)
{
	// 보편참조가 되기때문에 이방법을 써야한다.
	using DataType = std::remove_reference_t<T>; // 레퍼런스에 '&' 참조값 부호를 땐다. ex) int32& -> int32
	*reinterpret_cast<DataType*>(&_buffer[_pos]) = std::forward<DataType>(src); //보편참조에서는 forward를 사용.
	_pos += sizeof(T);
	return *this;
}
