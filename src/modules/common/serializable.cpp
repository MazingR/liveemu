#pragma once 

#include "serializable.hpp"

void ComputeSqlRefPropertyInsert(const FeTPtr<FeSerializable>* pValue, const char* sTypeName, const char* sName, char* output, uint32 iOutputLen, bool bInsert, uint32& iOutputedLen)
{
}
void ComputeSqlRefPropertyLink(const FeTPtr<FeSerializable>* pValue, const char* sTypeName, const char* sName, char* output, uint32 iOutputLen, bool bInsert, uint32& iOutputedLen)
{
}
void ComputeSqlPropertyName(const FeTPtr<FeSerializable>* pValue, const char* sTypeName, const char* sName, char* output, uint32 iOutputLen, bool bInsert, uint32& iOutputedLen)
{
	FE_ASSERT(false, "Do nothing");
}
void ComputeSqlPropertyValue(const FeTPtr<FeSerializable>* pValue, const char* sTypeName, const char* sName, char* output, uint32 iOutputLen, bool bInsert, uint32& iOutputedLen)
{
	FE_ASSERT(false, "Do nothing");
}

bool FetchProperty(FeSerializerValue& obj, FeSerializerValue& property, const char* sPropertyName)
{
	if (strcmp(sPropertyName, ".") == 0)
	{
		property = obj;
		return true;
	}
	else if (obj.HasMember(sPropertyName))
	{
		property = obj[sPropertyName];
		return true;
	}

	return false;
}

uint32 FeDeserialize(FeSerializerValue& value, bool* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FeSerializerValue jsonProperty;

	if (FetchProperty(value, jsonProperty, _sPropertyName))
		*pOutput = jsonProperty.GetBool();

	return FeEReturnCode::Success;
}

uint32 FeDeserialize(FeSerializerValue& value, int* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FeSerializerValue jsonProperty;

	if (FetchProperty(value, jsonProperty, _sPropertyName))
		*pOutput = jsonProperty.GetInt();

	return FeEReturnCode::Success;
}

uint32 FeDeserialize(FeSerializerValue& value, uint32* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FeSerializerValue jsonProperty;

	if (FetchProperty(value, jsonProperty, _sPropertyName))
		*pOutput = jsonProperty.GetUint();

	return FeEReturnCode::Success;
}

uint32 FeDeserialize(FeSerializerValue& value, float* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FeSerializerValue jsonProperty;

	if (FetchProperty(value, jsonProperty, _sPropertyName))
		*pOutput = (float)jsonProperty.GetDouble();

	return FeEReturnCode::Success;
}

uint32 FeDeserialize(FeSerializerValue& value, FeVector3* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FeSerializerValue jsonProperty;

	if (FetchProperty(value, jsonProperty, _sPropertyName))
		*pOutput = FeVector3(
		(float)jsonProperty[0].GetDouble(),
		(float)jsonProperty[1].GetDouble(),
		(float)jsonProperty[2].GetDouble());

	return FeEReturnCode::Success;
}

uint32 FeDeserialize(FeSerializerValue& value, FeColor* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FeSerializerValue jsonProperty;

	if (FetchProperty(value, jsonProperty, _sPropertyName))
		*pOutput = FeColor(
		(float)jsonProperty[0].GetDouble(),
		(float)jsonProperty[1].GetDouble(),
		(float)jsonProperty[2].GetDouble(),
		(float)jsonProperty[3].GetDouble());

	return FeEReturnCode::Success;
}

uint32 FeDeserialize(FeSerializerValue& value, FePath* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FeSerializerValue jsonProperty;

	if (FetchProperty(value, jsonProperty, _sPropertyName))
		pOutput->Set(jsonProperty.GetString());

	return FeEReturnCode::Success;
}

uint32 FeDeserialize(FeSerializerValue& value, FeTransform* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FeSerializerValue jsonProperty;

	if (FetchProperty(value, jsonProperty, _sPropertyName))
	{
		if (jsonProperty.HasMember("T"))
		{
			pOutput->Translation = FeVector3(
				(float)jsonProperty["T"][0].GetDouble(),
				(float)jsonProperty["T"][1].GetDouble(),
				(float)jsonProperty["T"][2].GetDouble());
		}
		if (jsonProperty.HasMember("R"))
		{
			pOutput->Rotation = FeRotation::EulerAngle(
				(float)jsonProperty["R"][0].GetDouble(),
				(float)jsonProperty["R"][1].GetDouble(),
				(float)jsonProperty["R"][2].GetDouble());
		}
		if (jsonProperty.HasMember("S"))
		{
			pOutput->Scale = FeVector3(
				(float)jsonProperty["S"][0].GetDouble(),
				(float)jsonProperty["S"][1].GetDouble(),
				(float)jsonProperty["S"][2].GetDouble());
		}
	}

	return FeEReturnCode::Success;
}

uint32 FeDeserialize(FeSerializerValue& value, FeString* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FeSerializerValue jsonProperty;

	if (FetchProperty(value, jsonProperty, _sPropertyName))
		*pOutput = FeStringPool::GetInstance()->CreateString(jsonProperty.GetString());

	return FeEReturnCode::Success;
}

// -------------------------------------------------------------------------------------------------------------------------

uint32 FeDeserializeDb(FeSerializerValue& value, bool* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FE_ASSERT(false, "Not implemented");
	return FeEReturnCode::Success;
}

uint32 FeDeserializeDb(FeSerializerValue& value, int* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FE_ASSERT(false, "Not implemented");
	return FeEReturnCode::Success;
}

uint32 FeDeserializeDb(FeSerializerValue& value, uint32* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FE_ASSERT(false, "Not implemented");
	return FeEReturnCode::Success;
}

uint32 FeDeserializeDb(FeSerializerValue& value, float* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FE_ASSERT(false, "Not implemented");
	return FeEReturnCode::Success;
}

uint32 FeDeserializeDb(FeSerializerValue& value, FeVector3* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FE_ASSERT(false, "Not implemented");
	return FeEReturnCode::Success;
}

uint32 FeDeserializeDb(FeSerializerValue& value, FeColor* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FE_ASSERT(false, "Not implemented");
	return FeEReturnCode::Success;
}

uint32 FeDeserializeDb(FeSerializerValue& value, FePath* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FE_ASSERT(false, "Not implemented");
	return FeEReturnCode::Success;
}

uint32 FeDeserializeDb(FeSerializerValue& value, FeTransform* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FE_ASSERT(false, "Not implemented");
	return FeEReturnCode::Success;
}

uint32 FeDeserializeDb(FeSerializerValue& value, FeString* pOutput, const char* _sPropertyName, uint32 iHeapId)
{
	FE_ASSERT(false, "Not implemented");
	return FeEReturnCode::Success;
}

uint32 FeSeserializeDb(char* output, uint32 outputLen, const bool&			input)
{
	return sprintf_s(output, outputLen, input ? "1" : "0");
}
uint32 FeSeserializeDb(char* output, uint32 outputLen, const int&			input)
{
	return sprintf_s(output, outputLen, "%d", input);
}
uint32 FeSeserializeDb(char* output, uint32 outputLen, const float&			input)
{
	return sprintf_s(output, outputLen, "%4.2f", input); 
}
uint32 FeSeserializeDb(char* output, uint32 outputLen, const uint32&		input)
{
	return sprintf_s(output, outputLen, "%u", input);
}
uint32 FeSeserializeDb(char* output, uint32 outputLen, const FePath&		input)
{
	return sprintf_s(output, outputLen, input.Value);
}
uint32 FeSeserializeDb(char* output, uint32 outputLen, const FeVector3&		input)
{
	//  todo
	return 0;
}
uint32 FeSeserializeDb(char* output, uint32 outputLen, const FeColor&		input)
{
	// todo
	return 0;
}
uint32 FeSeserializeDb(char* output, uint32 outputLen, const FeTransform&	input)
{
	// todo
	return 0;
}

uint32 FeSeserializeDb(char* output, uint32 outputLen, const FeString& intput)
{
	if (intput.IsEmpty())
		return 0;

	uint32 strLen = strlen(intput.Cstr());
	memcpy_s(output, outputLen, intput.Cstr(), strLen);
	output[strLen] = '\0';
	
	if (strLen)
		FeStringTools::Replace(output, '\'', ' ');

	return strLen;
}
