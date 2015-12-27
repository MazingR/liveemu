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

	//struct FeStringP
	//{
	//	size_t Length() const { return strlen(BaseAdress); }

	//	FeStringP& operator =(const char* str)
	//	{
	//		sprintf_s(BaseAdress, Size, str);
	//		return *this;
	//	}
	//	FeStringP& operator =(const FeStringP& str)
	//	{
	//		sprintf_s(BaseAdress, Size, str.Cstr());
	//		return *this;
	//	}

	//	const char* Cstr() const
	//	{
	//		return BaseAdress;
	//	}
	//private:
	//	void UpdateHash()
	//	{
	//		Hash = GenerateUIntIdFromString(BaseAdress, 0)
	//	}
	//	char* BaseAdress;
	//	uint32 Hash;
	//};

	template <size_t PoolSize, size_t StringSize>
	class FeStringPool
	{
		FeStringPool()
		{
			BaseAdress = FE_ALLOCATE_TYPED_ARRAY(char, PoolSize);
		}
		~FeStringPool()
		{
			FE_FREE_ARRAY(BaseAdress);
		}
		//FeStringP& Create(const char* str)
		//{

		//}
		//FeTArray<FeStringP<>> Strings;
		char* BaseAdress;
	};
}