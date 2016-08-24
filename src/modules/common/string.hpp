#pragma once 

#include <common.hpp>
#include <tarray.hpp>
#include <map>

namespace FeStringTools
{
	uint32 GenerateUIntIdFromString(const char* cptr, const char* cptr2=nullptr, unsigned int  _crc=0);
	void ToLower(char* szStr);
	void ToUpper(char* szStr);
	
	size_t IndexOf(const char* szString, char szChar, size_t iStart=0, size_t iEnd=0);
	size_t LastIndexOf(const char* szString, char szChar, size_t iStart = 0, size_t iEnd = 0);

	size_t IndexOf(const char* szString, const char* szFind, size_t iStart = 0, size_t iEnd =0);

	size_t Count(const char* szString, char szChar, size_t iStart = 0, size_t iEnd =0);
	size_t Replace(char* szString, char szFind, char szReplace);
	size_t TrimEnd(char* szString, char szChar);

	bool TrimLeft(char** szString, char* szTrimed, uint32 strLimit=0);
}

struct FePooledString
{
	uint32 Id;
	uint16 Size;
	char * Cstr;
	uint16 RefCount;
};
struct FeString
{
	friend class FeStringPool;

public:
	inline const char* Cstr() const { return Pooled ? Pooled->Cstr : nullptr; }
	inline const uint32 Id() const { return Pooled ? Pooled->Id : 0; }
	
	FeString() : Pooled(nullptr) {}
	FeString(FePooledString& pooledStr);
	FeString(const FeString& copy);
	FeString(const char* other);

	~FeString();

	FeString& operator=(const FeString& other);
	FeString& operator=(const char* other);
	
	bool IsEmpty() const
	{
		return !Pooled;
	}
	bool operator==(const FeString& other) const
	{
		return other.Pooled == this->Pooled;
	}
	void SetPooledStr(FePooledString* pooledStr)
	{
		if (Pooled)
			Pooled->RefCount--;
		
		Pooled = pooledStr;
		
		if (Pooled)
			Pooled->RefCount++;
	}

private:
	FePooledString* Pooled;
};

class FeStringPool
{
public:
	static FeStringPool* GetInstance();
	FeString CreateString(const char* szValue);
	void FeStringPool::CreateString(const char* szValue, FeString& output);
	void DeleteString(FeString* pStr);
private:
	FePooledString* CreatePooledString(const char* szValue);
	
	typedef std::map<uint32, FePooledString> StringPoolMap;
	typedef StringPoolMap::iterator StringPoolMapIt;

	std::map<uint32, FePooledString> Pool;
};
