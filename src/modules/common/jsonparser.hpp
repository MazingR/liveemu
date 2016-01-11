#pragma once 

#include <common.hpp>
#include "filesystem.hpp"

#pragma warning(disable: 4244)
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

typedef rapidjson::Value FeSerializerValue;

class FeSerializable
{
public:
	virtual uint32 Serialize(FeSerializerValue& serializer) 
	{
		return FeEReturnCode::Success;
	}
	virtual uint32 Deserialize(FeSerializerValue& serializer)
	{
		return FeEReturnCode::Success;
	}
};
typedef FeSerializable*(*FeCreateObjectFunc) ();

struct FeFactory
{
	char				TypeName[COMMON_STR_SIZE];
	FeCreateObjectFunc	CreateFunc;
};

class FeObjectsFactory
{
public:
	void RegisterFactory(const char* sTypeName, FeCreateObjectFunc createFunc);
	FeSerializable* CreateObjectFromFactory(const char* sTypeName);

	static FeObjectsFactory StaticInstance;
	typedef std::map<uint32, FeFactory> FactoriesMap;
	typedef FactoriesMap::iterator FactoriesMapIt;

	template<class T>
	void CreateFactory(FeCreateObjectFunc func)
	{
		if (!HasFactory())
		{
			uint32 iTypeHash = typeid(T).hash_code();
			char sTypeName[64] = typeid(T).name;

			{
				FeFactory newFactory;
				Factories.insert(iTypeHash, newFactory)
			}
			{
				FeFactory& newFactory = Factories[iTypeHash];
				newFactory.CreateFunc = func;
				sprintf_s(newFactory.TypeName, sTypeName);
			}
		}
	}
	template<class T>
	const FeFactory& HasFactory()
	{
		static uint32 iTypeHash = typeid(T).hash_code();
		return Factories.find(iTypeHash) != Factories.end();
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

}
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

		FeFileTools::ReadTextFile(path, &szContent, &iFileSize);

		rapidjson::Document d;
		rapidjson::ParseResult result = d.Parse(szContent);
		FE_FREE(szContent, 1);

		if (result.IsError())
			return FeEReturnCode::Failed;

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

	template<>
	uint32 DeserializeObject(FeSerializable*& output, FeSerializerValue& value)
	{
		return output->Deserialize(value);
	}

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