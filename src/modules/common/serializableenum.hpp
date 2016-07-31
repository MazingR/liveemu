#pragma once 

#include <common.hpp>
#include <jsonparser.hpp>
#include "maths.hpp"

// -------------------------------------------------------------------------------------------------------------------------
// Enum declaration preprocessing for serialization
// -------------------------------------------------------------------------------------------------------------------------
#define FE_DECLARE_ENUM_VALUE(_name_, _value_)	\
	_value_,									\


#define FE_DECLARE_ENUM_MAP_INJECTION(_name_, _value_)									\
	iValuesMap[FeStringTools::GenerateUIntIdFromString(#_value_)] = _name_::_value_;	\


#define FE_DECLARE_STRING_ENUM_MAP_ENTRY(_name_, _value_)								\
	_map_[FeStringPool::GetInstance()->CreateString( #_value_ ).Id()] = _name_::_value_;\


#define FE_DECLARE_STRING_ENUM_MAP(_name_, _values_)																\
	static _name_::Type _name_##FromString(const FeString& str)														\
	{																												\
		static std::map<uint32, _name_::Type> _map_;																\
		static bool bIsMapInit = false;																				\
																													\
		if (!bIsMapInit)																							\
				{																									\
			bIsMapInit = true;																						\
			_values_(FE_DECLARE_STRING_ENUM_MAP_ENTRY)																\
				}																									\
		FE_ASSERT(_map_.find(str.Id()) != _map_.end(), 																\
			"unknown enum value %s to map from enum %s !", str.Cstr(), #_name_);									\
		return _map_[str.Id()];																						\
	}																												\


#define FE_DECLARE_ENUM(_name_, _values_)																						\
	namespace _name_																											\
	{																															\
		enum Type																												\
		{																														\
			_values_(FE_DECLARE_ENUM_VALUE)																						\
			Count																												\
		};																														\
	};																															\
	static uint32 FeDeserialize(FeSerializerValue& value, _name_::Type* pOutput, const char* _sPropertyName, uint32 iHeapId)	\
	{																															\
		FeSerializerValue jsonProperty;																							\
		if (!FetchProperty(value, jsonProperty, _sPropertyName))																\
			return FeEReturnCode::Success;																						\
																																\
		static std::map<uint32, _name_::Type> iValuesMap;																		\
		if (iValuesMap.size() == 0)																								\
		{																														\
			_values_(FE_DECLARE_ENUM_MAP_INJECTION)																				\
		}																														\
																																\
		uint32 iValue = FeStringTools::GenerateUIntIdFromString(jsonProperty.GetString());										\
																																\
		if (iValuesMap.find(iValue) != iValuesMap.end())																		\
		{																														\
			*pOutput = iValuesMap[iValue];																						\
			return FeEReturnCode::Success;																						\
		}																														\
		else																													\
		{																														\
			FE_ASSERT(false, "Couldn't deserialize enum %s with value %s", #_name_, jsonProperty.GetString());					\
			return FeEReturnCode::Failed;																						\
		}																														\
	}																															\
	static uint32 FeDeserializeDb(FeSerializerValue& value, _name_::Type* pOutput, const char* _sPropertyName, uint32 iHeapId)	\
	{																															\
			FE_ASSERT(false, "Not implemented");																				\
			return FeEReturnCode::Failed;																						\
	}																															\
FE_DECLARE_STRING_ENUM_MAP(_name_, _values_)																					\

