#pragma once 

#include <common.hpp>
#include <tarray.hpp>

#include <unordered_set>

namespace FeStringTools
{
	uint32 GenerateUIntIdFromString(const char* cptr, const char* cptr2=NULL, unsigned int  _crc=0);
	unsigned int GenerateIntIdFromBuffer(const void * key, int len, unsigned int seed);
	void ToLower(char* szStr);
	void ToUpper(char* szStr);
	
	size_t IndexOf(const char* szString, char szChar, size_t iStart=0, size_t iEnd=(size_t)-1);
	size_t LastIndexOf(const char* szString, char szChar, size_t iStart = 0, size_t iEnd = (size_t)-1);
	size_t Count(const char* szString, char szChar, size_t iStart = 0, size_t iEnd = (size_t)-1);
	size_t Replace(char* szString, char szFind, char szReplace);
}