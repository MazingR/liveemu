#pragma once 

#include <common.hpp>

class FeDatabaseImpl;
class FeDatabase
{
public:
	static FeDatabase StaticInstance;

	~FeDatabase();

	uint32 Load(const FePath& path);
	uint32 Execute(const char* szExec);
	uint32 ExecuteInsert(const char* szExec, uint32& ID);
private:
	FeDatabaseImpl* Impl;
};
