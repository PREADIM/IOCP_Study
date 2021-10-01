#pragma once


class BufferWriter
{
public:
	BufferWriter();
	BufferWriter(BYTE* buffer, uint32 size, uint32 pos = 0);
	~BufferWriter();


	template<typename T>
	bool Write(T* src) { return Write(src, sizeof(T)); } // �� �����ϰ� �������� �Լ�. �̷� ������ִٴ°��� ����.
	bool Write(void* src, uint32 len); // ������ �ۼ�

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
	
	T* ret = reinterpret_cast<T*>(&_buffer[_pos]); // �ش� ������ �ְ�
	_pos += sizeof(T) * count; // Ŀ�� �ű�� 
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
	// ���������� �Ǳ⶧���� �̹���� ����Ѵ�.
	using DataType = std::remove_reference_t<T>; // ���۷����� '&' ������ ��ȣ�� ����. ex) int32& -> int32
	*reinterpret_cast<DataType*>(&_buffer[_pos]) = std::forward<DataType>(src); //�������������� forward�� ���.
	_pos += sizeof(T);
	return *this;
}
