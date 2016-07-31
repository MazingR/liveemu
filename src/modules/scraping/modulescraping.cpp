#include <modulescraping.hpp>

#include <common/serializable.hpp>
#include <common/maths.hpp>
#include <queue>


static int DbCallback(void *NotUsed, int argc, char **argv, char **azColName)
{
	//int i;
	//for (i = 0; i<argc; i++)
	//{
	//	printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "nullptr");
	//}
	//printf("\n");
	return 0;
}

class FeModuleScrapingImpl
{
public:
	FeModuleScrapingImpl()
	{
		SqlScript = FE_NEW_ARRAY(char, SqlScriptLen, 0);
	}
	~FeModuleScrapingImpl()
	{
		FE_DELETE_ARRAY(char, SqlScript, SqlScriptLen, 0);
	}
	template<typename FeDbSerializable>
	uint32 InsertOrUpdateEntry(FeDbSerializable* pEntry)
	{
		uint32 iOutputed = 0;
		uint32 RowID;
		memset(SqlScript, 0, SqlScriptLen);

		/*int(*callback)(void*, int, char**, char**) = [](void *NotUsed, int argc, char **argv, char **azColName) -> int
		{
			return 0;
		};*/
		
		// not from db, try to recover row ID from Db
		if (pEntry->GetID() == FE_INVALID_ID)
		{
			if (pEntry->HasSecondaryKey())
			{
				RowID = FeDatabase::StaticInstance.GetRowID(pEntry->ClassName, pEntry->GetSecondaryKey(), pEntry->GetName().Cstr());
				pEntry->SetID(RowID);
			}
			else
			{
				FE_ASSERT(false, "trying to insert invalid row id to db");
				return FeEReturnCode::Failed;
			}
		}

		// New entry
		if (pEntry->GetID() == FE_INVALID_ID)
		{
			pEntry->ComputeSqlInsert(SqlScript, SqlScriptLen, iOutputed);
			if (FE_SUCCEEDED(FeDatabase::StaticInstance.ExecuteInsert(SqlScript, RowID, DbCallback)))
				pEntry->SetID(RowID);
			else
				return FeEReturnCode::Failed;
		}
		else
		{
			pEntry->ComputeSqlUpdate(SqlScript, SqlScriptLen, iOutputed);
			return FeDatabase::StaticInstance.Execute(SqlScript, DbCallback);
		}
		return FeEReturnCode::Success;
	}
private:

	const uint32 SqlScriptLen = 2048;
	char* SqlScript;
};
uint32 FeModuleScraping::Load(const FeModuleInit* initBase)
{
	auto init = (FeModuleScrapingInit*)initBase;
	Impl = FE_NEW(FeModuleScrapingImpl, 0);
	
	//FeTArray<FeDataFile>			DataFiles;

	//FeTArray<FePath> dbFiles;
	//dbFiles.SetHeapId(2);
	//FeFileTools::ListFilesRecursive(dbFiles, "test/data", ".*\\.json");

	//DataFiles.SetHeapId(JSON_HEAP);
	//DataFiles.Clear();
	//DataFiles.Reserve(dbFiles.GetSize());

	//for (auto& file : dbFiles)
	//{
	//	FeDataFile& dataFile = DataFiles.Add();

	//	auto iRes = FeJsonParser::DeserializeObject(dataFile, file, JSON_HEAP);
	//	if (iRes != FeEReturnCode::Success)
	//		DataFiles.PopBack();
	//}

	FePath dbPath;
	dbPath.SetR("db/main.db");

	FeDatabase::StaticInstance.Load(dbPath);

	//const uint32 iSqlLen = 2048;

	//char* szSql = FE_NEW_ARRAY(char, iSqlLen, 0);

	//DataFiles.Add();

	FeTArray<FeDataGame> Games;
	FeTArray<FeDataPlatform> Platforms;
	FeTArray<FeDataGameGenre> GameGenres;
	
	char platforms[][32]
	{
		"Sega Genesis",
		"Arcade",
		"Super Nes",
		"Sega Saturn",
	};

	char genres[][32]
	{
		"Action",
		"Platform",
		"Shooter",
		"Sport",
		"Race",
	};
	char games[][2][32]
	{
		{ "Sonic",			"sonic c'est cool" },
		{ "Mario",			"mario tuyaux" },
		{ "Street Fighter",	"on saute pas sur le gros" },
		{ "Virtua Fighter",	"ora ora ora !" },
		{ "Panzer Dragoon",	"azel" },
	};

	uint32 iPlatformsCount = sizeof(platforms) / 32;
	for (uint32 i = 0; i < iPlatformsCount; ++i)
	{
		Platforms.Add();
		Platforms[i].SetName(platforms[i]);
	}

	uint32 iGenresCount = sizeof(genres) / 32;
	for (uint32 i = 0; i < iGenresCount;++i)
	{
		GameGenres.Add();
		GameGenres[i].SetName(genres[i]);
	}

	uint32 iGamesCount = sizeof(games) / (32*2);
	for (uint32 i = 0; i < iGamesCount; ++i)
	{
		Games.Add();
		Games[i].SetName(games[i][0]);
		Games[i].SetOverview(games[i][1]);

		Games[0].SetPlatform(FeTPtr<FeDataPlatform>(&Platforms[0]));
		Games[0].SetGenre(FeTPtr<FeDataGameGenre>(&GameGenres[0]));
	}

	for (auto& platform : Platforms)
		Impl->InsertOrUpdateEntry(&platform);
	for (auto& genre : GameGenres)
		Impl->InsertOrUpdateEntry(&genre);
	for (auto& game : Games)
		Impl->InsertOrUpdateEntry(&game);


	//uint32 iOutputed = 0;
	//uint32 ID = FE_INVALID_ID;

	//memset(szSql, 0, iSqlLen);
	//int(*callback)(void*, int, char**, char**) = [](void *NotUsed, int argc, char **argv, char **azColName) -> int
	//{
	//	return 0;
	//};
	//
	//ID = FeDatabase::StaticInstance.GetRowID(Platforms[0].ClassName, Platforms[0].GetSecondaryKey(), Platforms[0].GetName().Cstr());

	//Platforms[0].ComputeSqlInsert(szSql, iSqlLen, iOutputed);
	//if (FE_SUCCEEDED(FeDatabase::StaticInstance.ExecuteInsert(szSql, ID, callback)))
	//	Platforms[0].SetID(ID);

	//memset(szSql, 0, iSqlLen);
	//GameGenres[0].ComputeSqlInsert(szSql, iSqlLen, iOutputed);
	//if (FE_SUCCEEDED(FeDatabase::StaticInstance.ExecuteInsert(szSql, ID, callback)))
	//	GameGenres[0].SetID(ID);

	//memset(szSql, 0, iSqlLen);
	//Games[0].ComputeSqlInsert(szSql, iSqlLen, iOutputed);
	//if (FE_SUCCEEDED(FeDatabase::StaticInstance.ExecuteInsert(szSql, ID, callback)))
	//	Games[0].SetID(ID);

	//for (auto & dataFile : DataFiles)
	//{
	//	memset(szSql, 0, iSqlLen);

	//	for (auto& game : dataFile.GetGames())
	//	{
	//		FE_LOG("Game\t%s", game.GetTitle().Cstr());

	//		uint32 iOutputed = 0;
	//		game.ComputeSqlInsert(szSql, iSqlLen, iOutputed);

	//		uint32 ID;
	//		if (FE_FAILED(FeDatabase::StaticInstance.ExecuteInsert(szSql, ID)))
	//			continue;
	//		game.SetID(ID);
	//	}
	//}
	//FE_DELETE_ARRAY(char, szSql, iSqlLen, 0);

	return FeEReturnCode::Success;
}
uint32 FeModuleScraping::Unload()
{
	FE_DELETE(FeModuleScrapingImpl, Impl, 0);

	return FeEReturnCode::Success;
}
uint32 FeModuleScraping::Update(const FeDt& fDt)
{
	return FeEReturnCode::Success;
}
