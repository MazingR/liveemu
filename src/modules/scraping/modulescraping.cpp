#include <modulescraping.hpp>

#include <common/serializable.hpp>
#include <common/maths.hpp>
#include <queue>

// ----------------------------------------------------------------------------------------------------------------------------------------------------------
class FeGameScrapperArcadeHistoryImpl
{
public:
	void LoadDatFileInfos()
	{
		char* szContent;
		size_t iFileSize;
		FePath path;

		FeFileTools::ComputeFullPath(path, "scrappers/arcadehistory/history.dat");
		
		auto iResult = FeFileTools::ReadTextFile(path, &szContent, &iFileSize);
		
		if (iResult == FeEReturnCode::Success)
		{
			char* line = szContent;
			uint32 iLineLen = FeStringTools::IndexOf(line, '\r', 0, 2048);

			uint32 iReadChars = 0;

			auto readChar = [&]() 
			{
				line++;
				iReadChars++;
			};
			auto readLine = [&]()
			{
				iLineLen = FeStringTools::IndexOf(line, '\r', 0, 2048);

				if (iLineLen > iFileSize - iReadChars)
					iLineLen = iFileSize - iReadChars;

				line += iLineLen + 1;
				iReadChars += iLineLen + 1;

				if (line[0] == '\n')
					readChar(); // read '\n

				iLineLen = FeStringTools::IndexOf(line, '\r', 0, 2048);
			};
			
			char szParsedRomNames[512];
			char szParsedName[512];
			char szParsedRomName[32];
			
			GamesList.SetHeapId(FE_HEAPID_FILESYSTEM);
			GamesList.Reserve(20000);

			while (iReadChars < iFileSize)
			{
				uint32 iCharsLeft = iFileSize - iReadChars;

				char* parsedLine = line;
				if (FeStringTools::TrimLeft(&parsedLine, "$info=", 2048))
				{
					memset(szParsedRomNames, 0, 512);
					memset(szParsedName, 0, 512);
					
					memcpy_s(szParsedRomNames, 512, parsedLine, FeStringTools::IndexOf(parsedLine, '\r', 0, 2048) - 1); // line minus 1 char ','
					// jump 3 lines
					readLine();readLine();readLine();
					uint32 iNameLen = FeStringTools::IndexOf(line, " (c)", 0, iLineLen);
					if (iNameLen == iLineLen)
						iNameLen = FeStringTools::IndexOf(line, "(c)", 0, iLineLen);
					
					if (iNameLen == iLineLen)
						continue;

					memcpy_s(szParsedName, 512, line, iNameLen); // line minus 1 char '.'

					if (strlen(szParsedName) && strlen(szParsedRomNames))
					{
						FeDataGame& Game = GamesList.Add();
						Game.SetName(szParsedName);
						Game.GetData().New();

						uint32 iRomNamesOffset = 0;
						uint32 iRomNamesLen = strlen(szParsedRomNames);
						uint32 iRomIdx = 0;

						while (iRomNamesOffset < iRomNamesLen)
						{
							memset(szParsedRomName, 0, 32);
							uint32 nameLen = FeStringTools::IndexOf(szParsedRomNames + iRomNamesOffset, ',');
							memcpy_s(szParsedRomName, 32, szParsedRomNames+iRomNamesOffset, nameLen);
							iRomNamesOffset += nameLen+1;

							FeDataGameDump* pDump = Game.GetData().Ptr->NewDumpPtr(iRomIdx);
							pDump->SetName(szParsedRomName);
							iRomIdx++;

							if (iRomIdx > 4)
								break;
						}
					}
				}
				else
				{
					readLine();
				}
			}

			FE_FREE(szContent, FE_HEAPID_FILESYSTEM);
		}
	}
	void FetchRomFilesChecksum()
	{

	}
	void InjectInfosInDatabase()
	{

	}
	
private:
	FeNTArray<FeDataGame> GamesList;
};
uint32 FeGameScrapperArcadeHistory::Load()
{
	Impl = FE_NEW(FeGameScrapperArcadeHistoryImpl, 0);
	
	Impl->LoadDatFileInfos();
	Impl->FetchRomFilesChecksum();
	Impl->InjectInfosInDatabase();

	return FeEReturnCode::Success;
}
uint32 FeGameScrapperArcadeHistory::Unload()
{
	FE_DELETE(FeGameScrapperArcadeHistoryImpl, Impl, 0);
	return FeEReturnCode::Success;
}
uint32 FeGameScrapperArcadeHistory::Scrap(FeDataGame& Game)
{
	return FeEReturnCode::Success;
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------
class FeGameScrapperGiantBombImpl
{

};
uint32 FeGameScrapperGiantBomb::Load()
{
	Impl = FE_NEW(FeGameScrapperGiantBombImpl, 0);
	return FeEReturnCode::Success;
}
uint32 FeGameScrapperGiantBomb::Unload()
{
	FE_DELETE(FeGameScrapperGiantBombImpl, Impl, 0);
	return FeEReturnCode::Success;
}
uint32 FeGameScrapperGiantBomb::Scrap(FeDataGame& Game)
{
	return FeEReturnCode::Success;
}
// ----------------------------------------------------------------------------------------------------------------------------------------------------------
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
void FeModuleScraping::Test()
{

	//FeTArray<FeDataFile>			DataFiles;

	//FeTArray<FePath> dbFiles;
	//dbFiles.SetHeapId(2);
	//FeFileTools::ListFilesRecursive(dbFiles, "test/data", ".*\\.json");

	//DataFiles.SetHeapId(FE_HEAPID_JSONPARSER);
	//DataFiles.Clear();
	//DataFiles.Reserve(dbFiles.GetSize());

	//for (auto& file : dbFiles)
	//{
	//	FeDataFile& dataFile = DataFiles.Add();

	//	auto iRes = FeJsonParser::DeserializeObject(dataFile, file, FE_HEAPID_JSONPARSER);
	//	if (iRes != FeEReturnCode::Success)
	//		DataFiles.PopBack();
	//}

	FePath dbPath;
	FeFileTools::ComputeFullPath(dbPath, "db/main.db");

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
		{ "Sonic", "sonic c'est cool" },
		{ "Mario", "mario tuyaux" },
		{ "Street Fighter", "on saute pas sur le gros" },
		{ "Virtua Fighter", "ora ora ora !" },
		{ "Panzer Dragoon", "azel" },
	};

	uint32 iPlatformsCount = sizeof(platforms) / 32;
	for (uint32 i = 0; i < iPlatformsCount; ++i)
	{
		Platforms.Add();
		Platforms[i].SetName(platforms[i]);
	}

	uint32 iGenresCount = sizeof(genres) / 32;
	for (uint32 i = 0; i < iGenresCount; ++i)
	{
		GameGenres.Add();
		GameGenres[i].SetName(genres[i]);
	}

	uint32 iGamesCount = sizeof(games) / (32 * 2);
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

}
uint32 FeModuleScraping::Load(const FeModuleInit* initBase)
{
	auto init = (FeModuleScrapingInit*)initBase;
	Impl = FE_NEW(FeModuleScrapingImpl, 0);

	CreateScrapper<FeGameScrapperArcadeHistory>();
	CreateScrapper<FeGameScrapperGiantBomb>();

	GetScrapper<FeGameScrapperArcadeHistory>()->Load();
	GetScrapper<FeGameScrapperGiantBomb>()->Load();
	
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
