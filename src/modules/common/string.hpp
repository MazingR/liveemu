#pragma once 

#include <common.hpp>
#include <tarray.hpp>
#include <map>

namespace FeStringTools
{
	uint32 GenerateUIntIdFromString(const char* cptr, const char* cptr2=NULL, unsigned int  _crc=0);
	void ToLower(char* szStr);
	void ToUpper(char* szStr);
	
	size_t IndexOf(const char* szString, char szChar, size_t iStart=0, size_t iEnd=(size_t)-1);
	size_t LastIndexOf(const char* szString, char szChar, size_t iStart = 0, size_t iEnd = (size_t)-1);
	size_t Count(const char* szString, char szChar, size_t iStart = 0, size_t iEnd = (size_t)-1);
	size_t Replace(char* szString, char szFind, char szReplace);
	size_t TrimEnd(char* szString, char szChar);
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
	inline const char* Cstr() const { return Pooled ? Pooled->Cstr : NULL; }
	inline const uint32 Id() const { return Pooled ? Pooled->Id : 0; }
	
	FeString() : Pooled(NULL) {}
	~FeString();

	FeString& operator=(const FeString& other);
	FeString(FePooledString& pooledStr);
	FeString(const FeString& copy);
	bool IsEmpty() const
	{
		return !Pooled;
	}
	bool operator==(const FeString& other) const
	{
		return other.Pooled == this->Pooled;
	}
private:
	FePooledString* Pooled;
};

class FeStringPool
{
public:
	static FeStringPool* GetInstance();
	FeString CreateString(const char* szValue);
	void DeleteString(FeString* pStr);
private:
	typedef std::map<uint32, FePooledString> StringPoolMap;
	typedef StringPoolMap::iterator StringPoolMapIt;

	std::map<uint32, FePooledString> Pool;
};
