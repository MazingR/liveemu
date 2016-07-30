#include <modulescraping.hpp>

#include <common/serializable.hpp>
#include <common/maths.hpp>
#include <queue>

struct FeModuleScrapingInit : public FeModuleInit
{
public:
};


uint32 FeModuleUi::Load(const FeModuleInit* initBase)
{
	auto init = (FeModuleScrapingInit*)initBase;

	FeTArray<FeDataFile>			DataFiles;

	FeTArray<FePath> dbFiles;
	dbFiles.SetHeapId(2);
	FeFileTools::ListFilesRecursive(dbFiles, "test/data", ".*\\.json");

	DataFiles.SetHeapId(JSON_HEAP);
	DataFiles.Clear();
	DataFiles.Reserve(dbFiles.GetSize());

	for (auto& file : dbFiles)
	{
		FeDataFile& dataFile = DataFiles.Add();

		auto iRes = FeJsonParser::DeserializeObject(dataFile, file, JSON_HEAP);
		if (iRes != FeEReturnCode::Success)
			DataFiles.PopBack();
	}

	FePath dbPath;
	dbPath.SetR("db/main.db");

	FeDatabase::StaticInstance.Load(dbPath);

	const uint32 iSqlLen = 2048;

	char* szSql = FE_NEW_ARRAY(char, iSqlLen, 0);

	for (auto & dataFile : DataFiles)
	{
		memset(szSql, 0, iSqlLen);

		for (auto& game : dataFile.GetGames())
		{
			FE_LOG("Game\t%s", game.GetTitle().Cstr());

			uint32 iOutputed = 0;
			game.ComputeSqlInsert(szSql, iSqlLen, iOutputed);

			uint32 ID;
			if (FE_FAILED(FeDatabase::StaticInstance.ExecuteInsert(szSql, ID)))
				continue;
			game.SetID(ID);
		}
	}
	FE_DELETE_ARRAY(char, szSql, iSqlLen, 0);

	return FeEReturnCode::Success;
}
uint32 FeModuleUi::Unload()
{
	return FeEReturnCode::Success;
}
uint32 FeModuleUi::Update(const FeDt& fDt)
{
	return FeEReturnCode::Success;
}
