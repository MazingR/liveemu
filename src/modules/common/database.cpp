#pragma once 

#include <database.hpp>
#include <sqlite3.h>

FeDatabase FeDatabase::StaticInstance;

//static int callback(void *NotUsed, int argc, char **argv, char **azColName)
//{
//	int i;
//	for (i = 0; i<argc; i++)
//	{
//		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "nullptr");
//	}
//	printf("\n");
//	return 0;
//}

class FeDatabaseImpl
{
public:

	FeDatabaseImpl()
	{
		SqlDb = nullptr;
		IsDbLoaded = false;
	}
	uint32 Load(const FePath& path)
	{
		int rc;

		rc = sqlite3_open(path.Value, &SqlDb);
		if (rc)
		{
			FE_ASSERT(false, "Can't open database: %s\n", sqlite3_errmsg(SqlDb))
			sqlite3_close(SqlDb);

			return FeEReturnCode::Failed;
		}

		IsDbLoaded = true;
		
		return FeEReturnCode::Success;
	}

	uint32 ExecuteInsert(const char* szExec, uint32& ID, int(*callback)(void*, int, char**, char**))
	{
		uint32 iRes = Execute(szExec, callback);
		if (!FE_FAILED(iRes))
		{
			ID = (uint32)sqlite3_last_insert_rowid(SqlDb);
		}
		return iRes;
	}
	uint32 Execute(const char* szExec, int(*callback)(void*, int, char**, char**))
	{
		FE_ASSERT(IsDbLoaded, "Db not loaded !");

		char *zErrMsg = 0;
		int rc = sqlite3_exec(SqlDb, szExec, callback, 0, &zErrMsg);
		
		if (rc != SQLITE_OK)
		{
			FE_ASSERT(false, "SQL error: %s\n", zErrMsg);

			sqlite3_free(zErrMsg);
			sqlite3_close(SqlDb);

			return FeEReturnCode::Failed;
		}
		return FeEReturnCode::Success;
	}
	uint32 GetRowID(const char* sTable, const char* sSecondaryKey, const char* sValue)
	{
		static uint32 iID = 0;
		static char szSql[1024];
		memset(szSql, 0, 1024);

		iID = FE_INVALID_ID;

		int(*callback)(void*, int, char**, char**) = [](void *NotUsed, int argc, char **argv, char **azColName) -> int
		{
			if (argc==1)
				iID = (uint32)atoi(argv[0]);

			return argc == 1 ? 0 : 1;
		};
		sprintf_s(szSql, "SELECT ID FROM %s WHERE %s='%s'", sTable, sSecondaryKey, sValue);

		Execute(szSql, callback);

		return iID;
	}
	void Unload()
	{
		if (IsDbLoaded)
		{
			sqlite3_close(SqlDb);
			IsDbLoaded = false;
		}
	}

public:
	sqlite3 *SqlDb;
	bool IsDbLoaded;
};

FeDatabase::~FeDatabase()
{
	Impl->Unload();

	FE_DELETE(FeDatabaseImpl, Impl, 0);
}

uint32 FeDatabase::Load(const FePath& path)
{
	if (!Impl)
		Impl = FE_NEW(FeDatabaseImpl, 0);

	return Impl->Load(path);
}
uint32 FeDatabase::Execute(const char* szExec, int(*callback)(void*, int, char**, char**))
{
	return Impl ? Impl->Execute(szExec, callback) : FeEReturnCode::Failed;
}
uint32 FeDatabase::ExecuteInsert(const char* szExec, uint32& ID, int(*callback)(void*, int, char**, char**))
{
	return Impl ? Impl->ExecuteInsert(szExec, ID, callback) : FeEReturnCode::Failed;
}
uint32 FeDatabase::GetRowID(const char* sTable, const char* sSecondaryKey, const char* sValue)
{
	return Impl ? Impl->GetRowID(sTable, sSecondaryKey, sValue) : FeEReturnCode::Failed;
}