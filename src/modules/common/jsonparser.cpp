#pragma once 

#include <jsonparser.hpp>

FeObjectsFactory FeObjectsFactory::StaticInstance;

FeSerializable* FeObjectsFactory::CreateObjectFromFactory(const char* sTypeName)
{
	FeSerializable* pResult = NULL;
	uint32 iTypeHash = FeStringTools::GenerateUIntIdFromString(sTypeName);
	FactoriesMapIt it = Factories.find(iTypeHash);
	
	if (it != Factories.end())
	{
		return it->second.CreateFunc();
	}
	return NULL;
}