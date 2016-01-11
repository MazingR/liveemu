#pragma once 

#include <jsonparser.hpp>

namespace FeObject
{
	static FeTArray<FeFactory> Factories;

	void RegisterFactory(const char* sTypeName, FeCreateObjectFunc createFunc)
	{
		FeFactory& newFactory = Factories.Add();
		newFactory.CreateFunc = createFunc;
		sprintf_s(newFactory.TypeName, sTypeName);
	}
	FeSerializable* CreateObjectFromFactory(const char* sTypeName)
	{
		FeSerializable* pResult = NULL;

		for (auto factory : Factories)
		{
			if (strcmp(factory.TypeName, sTypeName) == 0)
			{
				pResult = factory.CreateFunc();

				break;
			}
		}

		return pResult;
	}
}