#pragma once
#include "DBConnection.h"


// DBBind�� DBConnection�� ���� �������� �ս��� ����Ҽ� �ֵ��� �����ִ� ���� Ŭ�����̴�.

template<int32 C>
struct FullBits { enum { value = (1 << (C - 1)) | FullBits<C-1>::value }; };

// FullBits<3> = value = ( 1 << 3 -1)) | value = ( 1 << 2 - 1 ) | value = 1 | value = 0
// 0000 0000 0000 0000 0000 0000 0000 0111 �� ����
// �״ϱ� �Ѹ���� ���ڷο� C�� ������ŭ ��Ʈ�� 1�� ä���.

template<>
struct FullBits<1> { enum { value = 1 }; };

template<>
struct FullBits<0> { enum { value = 0 }; };



//���ø� Ŭ���� �̱⶧���� �����ɶ����� ���� �ٸ� Ŭ���� �̴�.
template<int32 ParamCount, int32 ColumnCount>
class DBBind
{
public:
	DBBind(DBConnection& dbConnection, const WCHAR* query)
		: _dbConnection(dbConnection), _query(query) // DBConnection���� query ���ڿ� �����صα�.
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
		// ��Ʈ Ȯ��. ParamCount�� ��ǲ ����, ColumnCount�� �ƿ�ǲ ����
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
	//_paramIndex , _columnIndex��� SQLLEN �迭�� �ƿ� �Է��� ������ŭ �̸� �������� ���.

	template<typename T>
	void BindParam(int32 idx, T& value) // ���° �ε��� �������� value�� �޾��� ���̴�.
	{
		_dbConnection.BindParam(idx + 1, &value, &_paramIndex[idx]); // idx��° SQLLEN ���.
		_paramFlag |= (1LL << idx); // ��Ʈ �÷��׸� Ų�� 1LL << idx��
		// 1LL << idx�� 1LL �� long long int���ε� 64��Ʈ(8����Ʈ) �� int64���� ũ�Ⱑ ����.
		// ���� 1 << idx�� �ϸ� 1�� �⺻������ �׳� int�� (4����Ʈ) �̱⶧���� idx�� 32���� ��ġ������
		// �ڱ��� �� ��Ʈ�� �ѱ⶧���� ������ ����⶧����, 1LL �Ǵ� 1i64 ��� �Է��ؾ��Ѵ�.
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
	void BindCol(int32 idx, WCHAR(&value)[N]) // ���ø����� �迭�� �������� �ѹ��� �˼��ִ� ����.
	{
		_dbConnection.BindCol(idx + 1, value, N - 1, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	void BindCol(int32 idx, WCHAR* value, int32 len) // ���ڿ�
	{
		_dbConnection.BindCol(idx + 1, value, len - 1, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

	template<typename T, int32 N>
	void BindCol(int32 idx, T(&value)[N]) // ����Ʈ �迭 , T(&value)[N]�̷��� �ϸ� �迭 �������� �˼�����
	{
		_dbConnection.BindCol(idx + 1, value, size32(T) * N, &_columnIndex[idx]);
		_columnFlag |= (1LL << idx);
	}

protected:
	DBConnection&	_dbConnection;
	const WCHAR*	_query;
	SQLLEN			_paramIndex[ParamCount > 0 ? ParamCount : 1]; // ���ø� ParamCount�� ��Ÿ���� �ƴ� ������ Ÿ�ӿ� �����Ǳ⶧���� ������ ����.
	SQLLEN			_columnIndex[ColumnCount > 0 ? ColumnCount : 1];
	uint64			_paramFlag;
	uint64			_columnFlag;
};

