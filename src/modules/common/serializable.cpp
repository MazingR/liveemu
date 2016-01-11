#pragma once 

#include "serializable.hpp"
#include "maths.hpp"


uint32 FeSerializerHelper::Deserialize(FeSerializerValue& value, int* pOutput, const char* _sPropertyName)
{
	FeSerializerValue jsonProperty;

	if (FetchProperty(value, jsonProperty, _sPropertyName))
		*pOutput = jsonProperty.GetInt();

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