#pragma once 

#include <common.hpp>
#include "filesystem.hpp"
#include "string.hpp"

#if HOOK_MALLOC

	#undef malloc
	#undef calloc
	#undef realloc
	#undef free

	#include <malloc.h>
	#undef malloc
	#undef calloc
	#undef realloc
	#undef free

	#define malloc(size)		std_FeMallocHook(size, DEFAULT_HEAP)
	#define calloc(nmemb, size)	std_FeCallocHook(nmemb, size, DEFAULT_HEAP)
	#define realloc(ptr, size)	std_FeReallocHook(ptr, size, DEFAULT_HEAP)
	#define free(ptr)			std_FeFreeHook(ptr, DEFAULT_HEAP)

	#pragma warning(disable: 4244)
	#include <rapidjson/document.h>
	#include <rapidjson/writer.h>
	#include <rapidjson/stringbuffer.h>

	#undef malloc
	#undef calloc
	#undef realloc
	#undef free

	#define malloc(size)		FeMallocHook(size, DEFAULT_HEAP)
	#define calloc(nmemb, size)	FeCallocHook(nmemb, size, DEFAULT_HEAP)
	#define realloc(ptr, size)	FeReallocHook(ptr, size, DEFAULT_HEAP)
	#define free(ptr)			FeFreeHook(ptr, DEFAULT_HEAP)

#else
	#pragma warning(disable: 4244)
	#include <rapidjson/document.h>
	#include <rapidjson/writer.h>
	#include <rapidjson/stringbuffer.h>
#endif

typedef rapidjson::Value FeSerializerValue;
class FeSerializable;

typedef FeSerializable*(*FeCreateObjectFunc) ();

struct FeFactory
{
	char				TypeName[COMMON_STR_SIZE];
	FeCreateObjectFunc	CreateFunc;
};

class FeCObjectsFactory
{
public:
	void RegisterFactory(const char* sTypeName, FeCreateObjectFunc createFunc);
	FeSerializable* CreateObjectFromFactory(const char* sTypeName);

	typedef std::map<uint32, FeFactory> FactoriesMap;
	typedef FactoriesMap::iterator FactoriesMapIt;

	template<typename T>
	const char* GetTypeName()
	{
		return typeid(T).name()+6; // class Name
	}

	template<typename T>
	void CreateFactory(FeCreateObjectFunc func)
	{
		const char* sTypeName = GetTypeName<T>();
		uint32 iTypeHash = FeStringTools::GenerateUIntIdFromString(sTypeName);

		if (Factories.find(iTypeHash) == Factories.end())
		{
			FeFactory newFactory;
			newFactory.CreateFunc = func;
			sprintf_s(newFactory.TypeName, sTypeName);

			Factories[iTypeHash] = newFactory;
		}
	}
	
	template<class T>
	const FeFactory& GetFactory()
	{
		static uint32 iTypeHash = typeid(T).hash_code();
		Factories[iTypeHash]
		return iTypeHash;
	}
private:
	FactoriesMap Factories;

};

FeCObjectsFactory& GetObjectsFactory();

template<typename T>
struct FeTFactory
{
	static FeSerializable* CreateInstance() { return FE_NEW(T, 1); }

	FeTFactory()
	{
		GetObjectsFactory().CreateFactory<T>(&FeTFactory<T>::CreateInstance);
	}
};

namespace FeJsonParser
{
	template<class T>
	uint32 DeserializeObject(T& output, const FePath& path)
	{
		return DeserializeObject(output, path.Value);
	}
	
	template<class T>
	uint32 DeserializeObject(T& output, const char* path)
	{
		char* szContent;
		size_t iFileSize;

		auto iResult = FeFileTools::ReadTextFile(path, &szContent, &iFileSize);
		if (iResult != FeEReturnCode::Success)
			return iResult;

		rapidjson::Document d;
		rapidjson::ParseResult result = d.Parse(szContent);
		
		if (result.IsError())
		{
			size_t iErrorOffset = result.Offset();
			size_t iTextLen = strlen(szContent);
			size_t iLineEnd = FeStringTools::IndexOf(szContent, '\n', iErrorOffset);
			if ((size_t)-1 == iLineEnd)
				iLineEnd = iTextLen;

			size_t iLineStart = FeStringTools::LastIndexOf(szContent, '\n', 0, iErrorOffset)+1;
			size_t iLineCount = FeStringTools::Count(szContent, '\n', 0, iLineStart)+1;
			size_t iLineLen = iLineEnd - iLineStart -1;

			char* szLine = FE_NEW_ARRAYD(char, iLineLen+1);
			if (iLineLen>0 && (iLineStart + iLineLen)<iTextLen)
			{
				memcpy_s(szLine, iLineLen, szContent + iLineStart, iLineLen);
				szLine[iLineLen] = '\0';
			}
			FE_ASSERT(false, "Deserialize failed : '%s' at line %u\n===> '%s'", path, iLineCount, szLine);

			FE_DELETE_ARRAYD(char, szLine, iLineLen);

			FE_FREE(szContent, 1);
			return FeEReturnCode::Failed;
		}

		FE_FREE(szContent, 1);

		return DeserializeObject(output, d);
	}

	template<class T>
	uint32 DeserializeObject(T& output, FeSerializerValue& value)
	{
		FeSerializable* pOutput = dynamic_cast<FeSerializable*>(&output);
		FE_ASSERT(pOutput, "Trying to deserialize non FeSerializable type !");

		if (!pOutput)
			return FeEReturnCode::Failed;

		(*pOutput).Deserialize(value);

		return FeEReturnCode::Success;
	}

	//template<>
	//uint32 DeserializeObject(FeSerializable*& output, FeSerializerValue& value)
	//{
	//	return output->Deserialize(value);
	//}

	template<class T>
	uint32 SerializeObject(const T& input, const FePath& path)
	{
		return SerializeObject(input, path.Value);
	}
	
	template<class T>
	uint32 SerializeObject(const T& input, const char* path)
	{
		rapidjson::Document d;

		// todo : save object properties to document

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<StringBuffer> writer(buffer);
		bool bResult = d.Accept(writer);

		if (!bResult)
			return FeEReturnCode::Failed;

		FE_ASSERT(bResult, "Failed serializing to json %s", path);

		FE_FAILEDRETURN(FeFileTools::WriteTextFile(path, buffer.GetString()));

		return FeEReturnCode::Success;
	}
}

