#pragma once

#include <commonscraping.hpp>
#include <common/application.hpp>

struct FeModuleScrapingInit : public FeModuleInit
{
public:
};

class FeModuleScrapingImpl;

class FeModuleScraping : public FeModule
{
	friend class FeModuleScrapingImpl;
public:
	virtual uint32 Load(const FeModuleInit*) override;
	virtual uint32 Unload() override;
	virtual uint32 Update(const FeDt& fDt) override;
private:
	FeModuleScrapingImpl* Impl;
};