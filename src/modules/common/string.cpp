#pragma once 

#include "string.hpp"
#include "serializable.hpp"
#include <string>

#define FE_HEAPID_STRINGPOOL 5

static FeStringPool* StaticInstance = nullptr;

FeString::FeString(const FeString& other) : FeString()
{
	if (other.Pooled)
	{
		Pooled = other.Pooled;
		Pooled->RefCount++;
	}
}
FeString::FeString(const char* other) : FeString()
{
	FeStringPool::GetInstance()->CreateString(other, *this);
}
FeString::FeString(FePooledString& pooledStr) : FeString()
{
	Pooled = &pooledStr;
	pooledStr.RefCount++;
}
FeString& FeString::operator= (const FeString& other)
{
	if (Pooled)
		Pooled->RefCount--;

	if (other.Pooled)
	{
		Pooled = other.Pooled;
		Pooled->RefCount++;
	}
	return *this;
}
FeString& FeString::operator= (const char* other)
{
	FeStringPool::GetInstance()->CreateString(other, *this);
	return *this;
}

FeString::~FeString()
{
	FeStringPool::GetInstance()->DeleteString(this);
}
void FeStringPool::DeleteString(FeString* pStr)
{
	//return;

	if (pStr->Pooled)
	{
		FePooledString& pooledStr = *pStr->Pooled;
		if (pooledStr.Size > 64)
		{
			pStr->Pooled = nullptr;

			FE_ASSERT(pooledStr.RefCount > 0, "Invalid pooled string ref count !");
			if (pooledStr.RefCount == 0)
				return;

			pooledStr.RefCount--;

			if (pooledStr.RefCount == 0)
			{
				// delete pooled string from memory
				FE_DELETE_ARRAY(char, pooledStr.Cstr, pooledStr.Size, FE_HEAPID_STRINGPOOL);
				Pool.erase(pooledStr.Id);
			}
		}
	}
}
FePooledString* FeStringPool::CreatePooledString(const char* szValue)
{
	if (!szValue)
		return nullptr;

	size_t iLen = strlen(szValue);
	if (!iLen)
		return nullptr;

	uint32 iId = FeStringTools::GenerateUIntIdFromString(szValue);
	static StringPoolMapIt itEnd = Pool.end();

	if (Pool.find(iId) != itEnd)
	{
		return &Pool[iId];
	}

	// Create new pooled string
	Pool[iId] = FePooledString();

	FePooledString* pooledStr = &Pool[iId];

	iLen++; // add one char for end of str
	pooledStr->Cstr = FE_NEW_ARRAY(char, iLen, FE_HEAPID_STRINGPOOL);

	memcpy_s(pooledStr->Cstr, iLen, szValue, iLen);
	pooledStr->Cstr[iLen] = '\0';
	//sprintf_s(pooledStr->Cstr, iLen, szValue);

	pooledStr->Id = iId;
	pooledStr->Size = iLen;
	pooledStr->RefCount = 1;

	return pooledStr;
}
void FeStringPool::CreateString(const char* szValue, FeString& output)
{
	FePooledString* pooledStr = CreatePooledString(szValue);
	output.SetPooledStr(pooledStr);
}
FeString FeStringPool::CreateString(const char* szValue)
{
	return FeString(*CreatePooledString(szValue));
}
FeStringPool* FeStringPool::GetInstance()
{
	if (!StaticInstance)
	{
		StaticInstance = FE_NEW(FeStringPool, 0);
	}

	return StaticInstance;
}

namespace FeStringTools
{
	void ToLower(char* szStr)
	{
		size_t iLen = strlen(szStr);
		for (size_t i = 0; i < iLen; ++i)
		{
			szStr[i] = tolower(szStr[i]);
		}
	}
	void ToUpper(char* szStr)
	{
		size_t iLen = strlen(szStr);
		for (size_t i = 0; i < iLen; ++i)
		{
			szStr[i] = toupper(szStr[i]);
		}
	}
	
	//-----------------------------------------------------------------------------
	// MurmurHash2, by Austin Appleby

	// Note - This code makes a few assumptions about how your machine behaves -

	// 1. We can read a 4-byte value from any address without crashing
	// 2. sizeof(int) == 4

	// And it has a few limitations -

	// 1. It will not work incrementally.
	// 2. It will not produce the same results on little-endian and big-endian
	//    machines.

	unsigned int GenerateIntIdFromBuffer(const void * key, int len, unsigned int seed)
	{
		// 'm' and 'r' are mixing constants generated offline.
		// They're not really 'magic', they just happen to work well.

		const unsigned int m = 0x5bd1e995;
		const int r = 24;

		// Initialize the hash to a 'random' value

		unsigned int h = seed ^ len;

		// Mix 4 bytes at a time into the hash

		const unsigned char * data = (const unsigned char *)key;

		while (len >= 4)
		{
			//		unsigned int k = *(unsigned int *)data;
			unsigned int k = (data[0]) + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);

			k *= m;
			k ^= k >> r;
			k *= m;

			h *= m;
			h ^= k;

			data += 4;
			len -= 4;
		}

		// Handle the last few bytes of the input array

		switch (len)
		{
		case 3: h ^= data[2] << 16;
		case 2: h ^= data[1] << 8;
		case 1: h ^= data[0];
			h *= m;
		};

		// Do a few final mixes of the hash to ensure the last few
		// bytes are well-incorporated.

		h ^= h >> 13;
		h *= m;
		h ^= h >> 15;

		return h;
	}

	uint32 GenerateUIntIdFromString(const char* cptr, const char* cptr2, unsigned int  _crc)
	{
		if (!cptr)
			return 0;

		uint32 length = (uint32)strlen(cptr);
		uint32 crc = GenerateIntIdFromBuffer(cptr, length, _crc);

		return crc;
	}

	size_t Count(const char* szString, char szChar, size_t iStart, size_t iEnd)
	{
		size_t iLen = iEnd ? iEnd : strlen(szString);
		size_t iCount = 0;

		for (size_t i = iStart; i < iLen; ++i)
		{
			if (szString[i] == szChar)
				iCount++;
		}

		return iCount;
	}
	size_t DoIndexOf(const char* szString, char szChar, size_t iStart, size_t iEnd, bool bReverse)
	{
		size_t iLen = iEnd ? iEnd : strlen(szString);

		for (size_t i = bReverse ? iLen - 1 : iStart; bReverse ? (i >= iStart) : (i < iLen); bReverse ? (--i) : (++i))
		{
			if (szString[i] == szChar)
				return  i;

			if (bReverse&&i == 0)
				break;
		}

		return iLen;
	}
	size_t DoIndexOf(const char* szString, const char* szFind, size_t iStart, size_t iEnd, bool bReverse)
	{
		size_t iLen = iEnd ? iEnd : strlen(szString);
		
		size_t iFindIdx = 0;
		size_t iFindLen = strlen(szFind);

		if (iFindLen > iLen)
			return iLen;

		for (size_t i = bReverse ? iLen - 1 : iStart; bReverse ? (i >= iStart) : (i < iLen); bReverse ? (--i) : (++i))
		{
			if (szString[i] == szFind[iFindIdx])
			{
				iFindIdx++;
				if (iFindIdx == iFindLen)
					return i - iFindLen + 1;
			}

			if (bReverse&&i == 0)
				break;
		}

		return iLen;
	}
	size_t IndexOf(const char* szString, char szChar, size_t iStart, size_t iEnd)
	{
		return DoIndexOf(szString, szChar, iStart, iEnd, false);
	}
	size_t LastIndexOf(const char* szString, char szChar, size_t iStart, size_t iEnd)
	{
		return DoIndexOf(szString, szChar, iStart, iEnd, true);
	}
	size_t IndexOf(const char* szString, const char* szFind, size_t iStart, size_t iEnd)
	{
		return DoIndexOf(szString, szFind, iStart, iEnd, false);
	}
	size_t Replace(char* szString, char szFind, char szReplace)
	{
		size_t iLen = strlen(szString);
		size_t iFound = 0;

		for (size_t i = 0; i < iLen; ++i)
		{
			if (szString[i] == szFind)
			{
				szString[i] = szReplace;
			}
		}

		return iFound;
	}
	size_t TrimEnd(char* szString, char szChar)
	{
		size_t iLen = strlen(szString);
		size_t iNewSize = iLen;

		for (size_t i = iLen-1; i >= 0; --i)
		{
			if (szString[i] == szChar)
			{
				szString[i] = '\0';
			}
			else
			{
				iNewSize = i + 1;
				break;
			}
		}

		return iNewSize;
	}

	bool TrimLeft(char** szString, char* szTrimed, uint32 strLimit)
	{
		uint32 strLen = strLimit == 0 ? strlen(*szString) : strLimit;
		uint32 strTrimLen = strlen(szTrimed);

		if (strTrimLen > strLen)
			return false;

		for (uint32 i = 0; i < strTrimLen; ++i)
		{
			if ((*szString)[i] != szTrimed[i])
				return false;
		}

		*szString += strTrimLen;

		return true;
	}
}