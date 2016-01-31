#pragma once 

#include "serializable.hpp"

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
