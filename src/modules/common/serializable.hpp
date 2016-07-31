#pragma once 

#include <common.hpp>
#include <jsonparser.hpp>
#include "maths.hpp"

#include "pointer.hpp"
#include "serializablepreprocessor.hpp"
#include "serializableenum.hpp"

namespace FeESerializeFormat
{
	enum Type
	{
		Json,
		Database
	};
}

class FeSerializable
{
public:
	virtual void ComputeSqlPropertiesName(char* output, uint32 iOutputLen, uint32& iOutputedLen, bool bInsert)
	{
	}
	virtual void ComputeSqlPropertiesValues(char* output, uint32 iOutputLen, uint32& iOutputedLen, bool bInsert)
	{
	}
	virtual uint32 Serialize(FeSerializerValue& serializer, FeESerializeFormat::Type format = FeESerializeFormat::Json)
	{
		return FeEReturnCode::Success;
	}
	virtual uint32 Deserialize(FeSerializerValue& serializer, uint32 iHeapId, FeESerializeFormat::Type format = FeESerializeFormat::Json)
	{
		return FeEReturnCode::Success;
	}

};

// ---------------------------------------------------------------------------------------------------------------------
// Json deserialize
// ---------------------------------------------------------------------------------------------------------------------

bool FetchProperty(FeSerializerValue& obj, FeSerializerValue& property, const char* sPropertyName);

template<typename T>
uint32 FeDeserialize(FeSerializerValue& value, FeNTArray<T>* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FeSerializerValue jsonProperty;

	if (!FetchProperty(value, jsonProperty, _sPropertyName))
		return FeEReturnCode::Success;

	if (jsonProperty.IsArray())
	{
		size_t iSize = jsonProperty.Size();
		pOutput->Clear();
		pOutput->SetHeapId(iHeapId);
		pOutput->Reserve(iSize);

		for (size_t i = 0; i < iSize; ++i)
		{
			FeSerializerValue& element = jsonProperty[i];
			T& elementObj = pOutput->Add();

			FeDeserialize(element, &elementObj, ".", iHeapId);
		}
	}
	return FeEReturnCode::Success;
}
template<typename T>
uint32 FeDeserialize(FeSerializerValue& value, FeTPtr<T>* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FeSerializerValue jsonProperty;

	if (!FetchProperty(value, jsonProperty, _sPropertyName))
		return FeEReturnCode::Success;

	if (jsonProperty.HasMember("_serialize_type_"))
	{
		pOutput->Ptr = (T*)GetObjectsFactory().CreateObjectFromFactory(jsonProperty["_serialize_type_"].GetString(), iHeapId);
	}

	return FeJsonParser::DeserializeObject(*pOutput->Ptr, jsonProperty, iHeapId);
}
uint32 FeDeserialize(FeSerializerValue& value, bool*		pOutput, const char* _sPropertyName, uint32 iHeapId);
uint32 FeDeserialize(FeSerializerValue& value, int*			pOutput, const char* _sPropertyName, uint32 iHeapId);
uint32 FeDeserialize(FeSerializerValue& value, float*		pOutput, const char* _sPropertyName, uint32 iHeapId);
uint32 FeDeserialize(FeSerializerValue& value, uint32*		pOutput, const char* _sPropertyName, uint32 iHeapId);
uint32 FeDeserialize(FeSerializerValue& value, FePath*		pOutput, const char* _sPropertyName, uint32 iHeapId);
uint32 FeDeserialize(FeSerializerValue& value, FeVector3*	pOutput, const char* _sPropertyName, uint32 iHeapId);
uint32 FeDeserialize(FeSerializerValue& value, FeColor*		pOutput, const char* _sPropertyName, uint32 iHeapId);
uint32 FeDeserialize(FeSerializerValue& value, FeTransform*	pOutput, const char* _sPropertyName, uint32 iHeapId);
uint32 FeDeserialize(FeSerializerValue& value, FeString*	pOutput, const char* _sPropertyName, uint32 iHeapId);

// ---------------------------------------------------------------------------------------------------------------------
// Database serialize
// ---------------------------------------------------------------------------------------------------------------------

template<typename T>
uint32 FeSeserializeDb(char* output, uint32 outputLen, const T& input)
{
	FE_ASSERT(false, "Not implemented");
	return 0;
}

template<typename T>
uint32 FeSeserializeDb(char* output, uint32 outputLen, const FeNTArray<T>& input)
{
	//FE_ASSERT(false, "Not implemented");
	return 0;
}

template<typename T>
uint32 FeSeserializeDb(char* output, uint32 outputLen, const FeTPtr<T>& input)
{
	return 0;
}

uint32 FeSeserializeDb(char* output, uint32 outputLen, const bool&			input);
uint32 FeSeserializeDb(char* output, uint32 outputLen, const int&			input);
uint32 FeSeserializeDb(char* output, uint32 outputLen, const float&			input);
uint32 FeSeserializeDb(char* output, uint32 outputLen, const uint32&		input);
uint32 FeSeserializeDb(char* output, uint32 outputLen, const FePath&		input);
uint32 FeSeserializeDb(char* output, uint32 outputLen, const FeVector3&		input);
uint32 FeSeserializeDb(char* output, uint32 outputLen, const FeColor&		input);
uint32 FeSeserializeDb(char* output, uint32 outputLen, const FeTransform&	input);
uint32 FeSeserializeDb(char* output, uint32 outputLen, const FeString&		input);

// ---------------------------------------------------------------------------------------------------------------------
// Database deserialize
// ---------------------------------------------------------------------------------------------------------------------

template<typename T>
uint32 FeDeserializeDb(FeSerializerValue& value, FeNTArray<T>* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FE_ASSERT(false, "Not implemented");
	return FeEReturnCode::Success;
}

template<typename T>
uint32 FeDeserializeDb(FeSerializerValue& value, FeTPtr<T>* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FE_ASSERT(false, "Not implemented");
	return FeEReturnCode::Success;
}

uint32 FeDeserializeDb(FeSerializerValue& value, bool*			pOutput, const char* _sPropertyName, uint32 iHeapId);
uint32 FeDeserializeDb(FeSerializerValue& value, int*			pOutput, const char* _sPropertyName, uint32 iHeapId);
uint32 FeDeserializeDb(FeSerializerValue& value, float*			pOutput, const char* _sPropertyName, uint32 iHeapId);
uint32 FeDeserializeDb(FeSerializerValue& value, uint32*		pOutput, const char* _sPropertyName, uint32 iHeapId);
uint32 FeDeserializeDb(FeSerializerValue& value, FePath*		pOutput, const char* _sPropertyName, uint32 iHeapId);
uint32 FeDeserializeDb(FeSerializerValue& value, FeVector3*		pOutput, const char* _sPropertyName, uint32 iHeapId);
uint32 FeDeserializeDb(FeSerializerValue& value, FeColor*		pOutput, const char* _sPropertyName, uint32 iHeapId);
uint32 FeDeserializeDb(FeSerializerValue& value, FeTransform*	pOutput, const char* _sPropertyName, uint32 iHeapId);
uint32 FeDeserializeDb(FeSerializerValue& value, FeString*		pOutput, const char* _sPropertyName, uint32 iHeapId);
