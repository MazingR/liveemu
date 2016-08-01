#pragma once 

#include <common.hpp>

class FeDatabaseImpl;
class FeDatabase
{
public:
	static FeDatabase StaticInstance;

	~FeDatabase();

	uint32 Load(const FePath& path);
	uint32 Execute(const char* szExec, int(*callback)(void*, int, char**, char**) = nullptr, void* userData = nullptr);
	uint32 ExecuteInsert(const char* szExec, uint32& ID, int(*callback)(void*, int, char**, char**) = nullptr);
	uint32 GetRowID(const char* sTable, const char* sSecondaryKey, const char* sValue);
private:
	FeDatabaseImpl* Impl;
};
