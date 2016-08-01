#pragma once

#include <commonscraping.hpp>
#include <common/application.hpp>

class FeModuleScraping;

struct FeModuleScrapingInit : public FeModuleInit
{
public:
};

class FeIGameScrapper
{
public:
	virtual uint32 Load(FeModuleScraping* module) = 0;
	virtual uint32 Unload() = 0;
	virtual uint32 Scrap(FeDataGame& Game) = 0;
	virtual void FeedDatabase(FeModuleScraping* module) = 0;
};
// -----------------------------------------------------------------------------
class FeGameScrapperArcadeHistoryImpl;
class FeGameScrapperArcadeHistory : public FeIGameScrapper
{
public:
	virtual uint32 Load(FeModuleScraping* module);
	virtual uint32 Unload();
	virtual uint32 Scrap(FeDataGame& Game);
	virtual void FeedDatabase(FeModuleScraping* module);
private:
	FeGameScrapperArcadeHistoryImpl* Impl;
};
// -----------------------------------------------------------------------------
class FeGameScrapperGiantBombImpl;
class FeGameScrapperGiantBomb : public FeIGameScrapper
{
public:
	virtual uint32 Load(FeModuleScraping* module);
	virtual uint32 Unload();
	virtual uint32 Scrap(FeDataGame& Game);
	virtual void FeedDatabase(FeModuleScraping* module);
private:
	FeGameScrapperGiantBombImpl* Impl;
};
// -----------------------------------------------------------------------------
class FeModuleScrapingImpl;
class FeModuleScraping : public FeModule
{
	friend class FeModuleScrapingImpl;
	typedef std::map<size_t, FeIGameScrapper*> ScrappersMap;
	typedef ScrappersMap::iterator ScrappersMapIt;

public:
	virtual uint32 Load(const FeModuleInit*) override;
	virtual uint32 Unload() override;
	virtual uint32 Update(const FeDt& fDt) override;

	template<class T>
	T* CreateScrapper()
	{
		T* pScrapper = FE_NEW(T, 0);
		Scrappers[GetScrapperID<T>()] = pScrapper;
		return pScrapper;
	}

	template<class T>
	size_t GetScrapperID()
	{
		static uint32 iTypeHash = typeid(T).hash_code();
		return iTypeHash;
	}
	template<class T>
	T* GetScrapper()
	{
		return (T*)Scrappers[GetScrapperID<T>()];
	}

	template<typename T>
	FeString* ComputeEntrySecondaryKeyValue(T* pEntry)
	{
		return (FeString*)pEntry->GetPropertyValueByName(pEntry->GetSecondaryKey());
	}
	template<typename T>
	uint32 FetchEntryRowID(T* pEntry)
	{
		uint32 RowID;

		if (pEntry->GetID() == FE_INVALID_ID)
		{
			if (pEntry->HasSecondaryKey())
			{
				FeString* pKeyValue = ComputeEntrySecondaryKeyValue(pEntry);

				if (pKeyValue)
				{
					RowID = FeDatabase::StaticInstance.GetRowID(pEntry->ClassName(), pEntry->GetSecondaryKey(), pKeyValue->Cstr());
					pEntry->SetID(RowID);
				}
			}
			else
			{
				FE_ASSERT(false, "trying to insert invalid row id to db");
				return FeEReturnCode::Failed;
			}
		}
		return RowID;
	}
	template<typename T>
	uint32 InsertOrUpdateEntry(T* pEntry)
	{
		// not from db, try to recover row ID from Db
		uint32 RowID = FetchEntryRowID(pEntry);

		uint32 iOutputed = 0;
		memset(SqlScript, 0, SqlScriptLen);
		// New entry
		if (pEntry->GetID() == FE_INVALID_ID)
		{
			pEntry->ComputeSqlInsert(SqlScript, SqlScriptLen, iOutputed);
			if (FE_SUCCEEDED(FeDatabase::StaticInstance.ExecuteInsert(SqlScript, RowID)))
				pEntry->SetID(RowID);
			else
				return FeEReturnCode::Failed;
		}
		else
		{
			pEntry->ComputeSqlUpdate(SqlScript, SqlScriptLen, iOutputed);
			return FeDatabase::StaticInstance.Execute(SqlScript);
		}
		return FeEReturnCode::Success;
	}
private:
	const uint32 SqlScriptLen = 2048;
	char* SqlScript;

	ScrappersMap Scrappers;
	FeModuleScrapingImpl* Impl;
};
