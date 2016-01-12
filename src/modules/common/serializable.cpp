#pragma once 

#include "serializable.hpp"

uint32 FeSerializerHelper::Deserialize(FeSerializerValue& value, int* pOutput, const char* _sPropertyName)
{
	FeSerializerValue jsonProperty;

	if (FetchProperty(value, jsonProperty, _sPropertyName))
		*pOutput = jsonProperty.GetInt();

	return FeEReturnCode::Success;
}

uint32 FeSerializerHelper::Deserialize(FeSerializerValue& value, uint32* pOutput, const char* _sPropertyName)
{
	FeSerializerValue jsonProperty;

	if (FetchProperty(value, jsonProperty, _sPropertyName))
		*pOutput = jsonProperty.GetUint();

	return FeEReturnCode::Success;
}

uint32 FeSerializerHelper::Deserialize(FeSerializerValue& value, float* pOutput, const char* _sPropertyName)
{
	FeSerializerValue jsonProperty;

	if (FetchProperty(value, jsonProperty, _sPropertyName))
		*pOutput = (float)jsonProperty.GetDouble();

	return FeEReturnCode::Success;
}

uint32 FeSerializerHelper::Deserialize(FeSerializerValue& value, FeVector3* pOutput, const char* _sPropertyName)
{
	FeSerializerValue jsonProperty;

	if (FetchProperty(value, jsonProperty, _sPropertyName))
		*pOutput = FeVector3(
		(float)jsonProperty[0].GetDouble(),
		(float)jsonProperty[1].GetDouble(),
		(float)jsonProperty[2].GetDouble());

	return FeEReturnCode::Success;
}

uint32 FeSerializerHelper::Deserialize(FeSerializerValue& value, FeColor* pOutput, const char* _sPropertyName)
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

uint32 FeSerializerHelper::Deserialize(FeSerializerValue& value, FePath* pOutput, const char* _sPropertyName)
{
	FeSerializerValue jsonProperty;

	if (FetchProperty(value, jsonProperty, _sPropertyName))
		pOutput->Set(jsonProperty.GetString());

	return FeEReturnCode::Success;
}

uint32 FeSerializerHelper::Deserialize(FeSerializerValue& value, FeTransform* pOutput, const char* _sPropertyName)
{
	FeSerializerValue jsonProperty;

	if (FetchProperty(value, jsonProperty, _sPropertyName))
	{
		pOutput->Translation = FeVector3(
			(float)jsonProperty["T"][0].GetDouble(),
			(float)jsonProperty["T"][1].GetDouble(),
			(float)jsonProperty["T"][2].GetDouble());
		pOutput->Rotation = FeRotation::EulerAngle(
			(float)jsonProperty["R"][0].GetDouble(),
			(float)jsonProperty["R"][1].GetDouble(),
			(float)jsonProperty["R"][2].GetDouble());
		pOutput->Scale = FeVector3(
			(float)jsonProperty["S"][0].GetDouble(),
			(float)jsonProperty["S"][1].GetDouble(),
			(float)jsonProperty["S"][2].GetDouble());
	}

	return FeEReturnCode::Success;
}