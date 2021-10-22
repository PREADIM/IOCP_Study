#pragma once
#include "DBConnection.h"
#include "DBModel.h"

/*--------------------
	DBSynchronizer
---------------------*/

class DBSynchronizer
{
	enum
	{
		PROCEDURE_MAX_LEN = 10000
	};

	enum UpdateStep : uint8 // 업데이트 순서 안전한 순서이다.
	{
		DropIndex,
		AlterColumn,
		AddColumn,
		CreateTable,
		DefaultConstraint,
		CreateIndex,
		DropColumn,
		DropTable,
		StoredProcecure,

		Max
	};

	enum ColumnFlag : uint8
	{
		Type = 1 << 0,
		Nullable = 1 << 1,
		Identity = 1 << 2,
		Default = 1 << 3,
		Length = 1 << 4,
	};

public:
	DBSynchronizer(DBConnection& conn) : _dbConn(conn) { }
	~DBSynchronizer();

	bool		Synchronize(const WCHAR* path);

private:
	void		ParseXmlDB(const WCHAR* path);
	bool		GatherDBTables();
	bool		GatherDBIndexes();
	bool		GatherDBStoredProcedures();

	void		CompareDBModel();
	void		CompareTables(DBModel::TableRef dbTable, DBModel::TableRef xmlTable);
	void		CompareColumns(DBModel::TableRef dbTable, DBModel::ColumnRef dbColumn, DBModel::ColumnRef xmlColumn);
	void		CompareStoredProcedures();

	void		ExecuteUpdateQueries();

private:
	DBConnection& _dbConn;

	Vector<DBModel::TableRef>			_xmlTables; // 이 둘이
	Vector<DBModel::ProcedureRef>		_xmlProcedures; // XML에서 설계한 상태
	Set<String>							_xmlRemovedTables; // 바로 테이블을 삭제하면 위험하기때문에 삭제를 위한 변수 사용x

	Vector<DBModel::TableRef>			_dbTables; // 이 둘이
	Vector<DBModel::ProcedureRef>		_dbProcedures; // DB의 현재 상태

	// 서로 둘이 비교하면서 다른점을 체크한다.

private:
	Set<String>							_dependentIndexes;
	Vector<String>						_updateQueries[UpdateStep::Max]; // 중간중간에 업데이트 해야할 퀴리문들 추가
};