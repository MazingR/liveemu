#pragma once

#include <commonscraping.hpp>
#include <common/application.hpp>

struct FeModuleScrapingInit : public FeModuleInit
{
public:
};

class FeIGameScrapper
{
public:
	virtual uint32 Load() = 0;
	virtual uint32 Unload() = 0;
	virtual uint32 Scrap(FeDataGame& Game) = 0;
};
// -----------------------------------------------------------------------------
class FeGameScrapperArcadeHistoryImpl;
class FeGameScrapperArcadeHistory : public FeIGameScrapper
{
public:
	virtual uint32 Load();
	virtual uint32 Unload();
	virtual uint32 Scrap(FeDataGame& Game);
private:
	FeGameScrapperArcadeHistoryImpl* Impl;
};
// -----------------------------------------------------------------------------
class FeGameScrapperGiantBombImpl;
class FeGameScrapperGiantBomb : public FeIGameScrapper
{
public:
	virtual uint32 Load();
	virtual uint32 Unload();
	virtual uint32 Scrap(FeDataGame& Game);
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
	void Test();

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
private:
	ScrappersMap Scrappers;
	FeModuleScrapingImpl* Impl;
};