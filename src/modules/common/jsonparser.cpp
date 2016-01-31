#pragma once 

#include <jsonparser.hpp>

FeCObjectsFactory FeObjectsFactory;

FeSerializable* FeCObjectsFactory::CreateObjectFromFactory(const char* sTypeName, uint32 iHeapId)
{
	FeSerializable* pResult = NULL;
	uint32 iTypeHash = FeStringTools::GenerateUIntIdFromString(sTypeName);
	FactoriesMapIt it = Factories.find(iTypeHash);
	
	if (it != Factories.end())
	{
		return it->second.CreateFunc(iHeapId);
	}
	return NULL;
}

FeCObjectsFactory& GetObjectsFactory()
{
	static FeCObjectsFactory FeObjectsFactory;

	return FeObjectsFactory;
}