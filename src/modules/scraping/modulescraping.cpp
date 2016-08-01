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
		
		uint32 iDebugGamesCountLimit = 50;

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
			GamesDataList.SetHeapId(FE_HEAPID_FILESYSTEM);
			GamesDumpList.SetHeapId(FE_HEAPID_FILESYSTEM);

			GamesList.Reserve(20000);
			GamesDataList.Reserve(20000);
			GamesDumpList.Reserve(8*20000);

			while (iReadChars < iFileSize)
			{
				if (GamesList.GetSize() == iDebugGamesCountLimit)
					break;

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
						FeStringTools::Replace(szParsedName, '\'', ' ');

						Game.SetName(szParsedName);
						Game.GetData().Assign(&GamesDataList.Add());

						uint32 iRomNamesOffset = 0;
						uint32 iRomNamesLen = strlen(szParsedRomNames);
						uint32 iRomIdx = 0;

						while (iRomNamesOffset < iRomNamesLen)
						{
							memset(szParsedRomName, 0, 32);
							uint32 nameLen = FeStringTools::IndexOf(szParsedRomNames + iRomNamesOffset, ',');
							memcpy_s(szParsedRomName, 32, szParsedRomNames+iRomNamesOffset, nameLen);
							iRomNamesOffset += nameLen+1;

							FeTPtr<FeDataGameDump>& Dump = Game.GetData().Get()->GetDump(iRomIdx);
							Dump.Assign(&GamesDumpList.Add());
							Dump.Get()->SetName(szParsedRomName);

							iRomIdx++;

							if (iRomIdx >= FeDataGameData::MaxDumpsCount)
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
	void InjectInfosInDatabase(FeModuleScraping* module)
	{
		FeDataPlatform arcade;
		arcade.SetName("Arcade");

		module->InsertOrUpdateEntry(&arcade);

		for (auto& game : GamesList)
		{
			module->InsertOrUpdateEntry(&game);
		}
	}
	
private:
	FeNTArray<FeDataGame> GamesList;
	FeNTArray<FeDataGameData> GamesDataList;
	FeNTArray<FeDataGameDump> GamesDumpList;
};
uint32 FeGameScrapperArcadeHistory::Load(FeModuleScraping* module)
{
	Impl = FE_NEW(FeGameScrapperArcadeHistoryImpl, 0);
	
	Impl->LoadDatFileInfos();
	Impl->FetchRomFilesChecksum();
	Impl->InjectInfosInDatabase(module);

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
uint32 FeGameScrapperGiantBomb::Load(FeModuleScraping* module)
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
template<typename TableType, typename PropertyType>
void FetchListEntriesPropertyFromDb(const char* szPropertyName, FeTArray<PropertyType>& Result)
{
	char szSql[512];
	memset(szSql, 0, 512);
	sprintf_s(szSql, "SELECT %s FROM %s", szPropertyName, TableType::ClassName());

	int(*callback)(void*, int, char**, char**) = [](void *userData, int argc, char **argv, char **azColName) -> int
	{
		auto results = (FeTArray<PropertyType>*)userData;
		if (argc == 1)
		{
			PropertyType& result = results->Add();
			result = argv[0];
		}
		return 0;
	};
	FeDatabase::StaticInstance.Execute(szSql, callback, &Result);
}
uint32 FeModuleScraping::Load(const FeModuleInit* initBase)
{
	auto init = (FeModuleScrapingInit*)initBase;
	
	// Load database
	FePath dbPath;
	FeFileTools::ComputeFullPath(dbPath, "db/main.db");
	FeDatabase::StaticInstance.Load(dbPath);

	SqlScript = FE_NEW_ARRAY(char, SqlScriptLen, 0);

	CreateScrapper<FeGameScrapperArcadeHistory>();
	CreateScrapper<FeGameScrapperGiantBomb>();

	GetScrapper<FeGameScrapperArcadeHistory>()->Load(this);
	GetScrapper<FeGameScrapperGiantBomb>()->Load(this);

	FeTArray<FeString> gameNames;
	FetchListEntriesPropertyFromDb<FeDataGame>("Name", gameNames);

	//for (auto& gameName : gameNames)
	//{
	//	FE_LOG("%s", gameName.Cstr());
	//}
	
	return FeEReturnCode::Success;
}
uint32 FeModuleScraping::Unload()
{
	FE_DELETE_ARRAY(char, SqlScript, SqlScriptLen, 0);

	return FeEReturnCode::Success;
}
uint32 FeModuleScraping::Update(const FeDt& fDt)
{
	return FeEReturnCode::Success;
}

FeString* ComputeEntrySecondaryKeyValue(FeDbSerializableNamed* pEntry)
{
	return &pEntry->GetName();
}