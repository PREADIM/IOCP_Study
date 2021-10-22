#pragma once
#include "DBConnection.h"


// DBBind는 DBConnection을 보다 서버에서 손쉽게 사용할수 있도록 도와주는 헬퍼 클래스이다.

template<int32 C>
struct FullBits { enum { value = (1 << (C - 1)) | FullBits<C-1>::value }; };

// FullBits<3> = value = ( 1 << 3 -1)) | value = ( 1 << 2 - 1 ) | value = 1 | value = 0
// 0000 0000 0000 0000 0000 0000 0000 0111 인 상태
// 그니까 한마디로 인자로온 C의 갯수만큼 비트를 1로 채운다.

template<>
struct FullBits<1> { enum { value = 1 }; };

template<>
struct FullBits<0> { enum { value = 0 }; };



//템플릿 클래스 이기때문에 생성될때마다 각각 다른 클래스 이다.
template<int32 ParamCount, int32 ColumnCount>
class DBBind
{
public:
	DBBind(DBConnection& dbConnection, const WCHAR* query)
		: _dbConnection(dbConnection), _query(query) // DBConnection형과 query 문자열 저장해두기.
	{
		::memset(_paramIndex, 0, sizeof(_paramIndex));
		::memset(_columnIndex, 0, sizeof(_columnIndex));
		_paramFlag = 0;
		_columnFlag = 0;
		dbConnection.Unbind();
	}

	bool Validate()
	{
		return _paramFlag == FullBits<ParamCount>::value && _columnFlag == FullBits<ColumnCount>::value;
		// 비트 확인. ParamCount는 인풋 갯수, ColumnCount는 아웃풋 갯수
	}

	bool Execute()
	{
		ASSERT_CRASH(Validate());
		return _dbConnection.Execute(_query);
	}

	bool Fetch()
	{
		return _dbConnection.Fetch();
	}

public:
	//_paramIndex , _columnIndex라는 SQLLEN 배열을 아예 입력한 갯수만큼 미리 만들어놓고 사용.

	template<typename T>
	void BindParam(int32 idx, T& value) // 몇번째 인덱스 기준으로 value를 받아줄 것이다.
	{
		_dbConnection.BindParam(idx + 1, &value, &_paramIndex[idx]); // idx번째 SQLLEN 사용.
		_paramFlag |= (1LL << idx); // 비트 플래그를 킨다 1LL << idx로
		// 1LL << idx는 1LL 은 long long int형인데 64비트(8바이트) 즉 int64형과 크기가 같다.
		// 보통 1 << idx를 하면 1은 기본적으로 그냥 int형 (4바이트) 이기때문에 idx에 32같은 수치가오면
		// 자기의 총 비트를 넘기때문에 오류가 생기기때문에, 1LL 또는 1i64 라고 입력해야한다.
	}

	void BindParam(int32 idx, const WCHAR* value)
	{
		_dbConnection.BindParam(idx + 1, value, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

	template<typename T, int32 N>
	void BindParam(int32 idx, T(&value)[N])
	{
		_dbConnection.BindParam(idx + 1, (const BYTE*)value, size32(T) * N, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

	template<typename T>
	void BindParam(int32 idx, T* value, int32 N)
	{
		_dbConnection.BindParam(idx + 1, (const BYTE*)value, size32(T) * N, &_paramIndex[idx]);
		_paramFlag |= (1LL << idx);
	}

	template<typename T>
	void BindCol(int32 idx, T& value)
	{
		_dbConnection.BindCol(idx + 1, &value, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	template<int32 N>
	void BindCol(int32 idx, WCHAR(&value)[N]) // 템플릿에서 배열의 갯수까지 한번에 알수있는 문법.
	{
		_dbConnection.BindCol(idx + 1, value, N - 1, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	void BindCol(int32 idx, WCHAR* value, int32 len) // 문자열
	{
		_dbConnection.BindCol(idx + 1, value, len - 1, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	template<typename T, int32 N>
	void BindCol(int32 idx, T(&value)[N]) // 바이트 배열 , T(&value)[N]이렇게 하면 배열 갯수까지 알수있음
	{
		_dbConnection.BindCol(idx + 1, value, size32(T) * N, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

protected:
	DBConnection&	_dbConnection;
	const WCHAR*	_query;
	SQLLEN			_paramIndex[ParamCount > 0 ? ParamCount : 1]; // 템플릿 ParamCount가 런타임이 아닌 컴파일 타임에 결정되기때문에 가능한 문법.
	SQLLEN			_columnIndex[ColumnCount > 0 ? ColumnCount : 1];
	uint64			_paramFlag;
	uint64			_columnFlag;
};

