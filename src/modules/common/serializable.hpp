#pragma once 

#include <common.hpp>
#include <jsonparser.hpp>
#include "maths.hpp"

// -------------------------------------------------------------------------------------------------------------------------
// Class declaration preprocessing for serialization
// -------------------------------------------------------------------------------------------------------------------------
/// <summary>
/// Step 1 : Simply declare the class members
/// </summary>
#define DECLARE_PROPERTY(t,n)				t n;
#define DECLARE_CLASS_MEMBERS(properties)		\
	properties(DECLARE_PROPERTY)

/// <summary>
/// Step 2 : Declare the function to serialize class members to json
/// </summary>
#define BEGIN_DECLARE_SERIALIZER(baseClass, thisClass)				\
	virtual uint32 Serialize(FeSerializerValue& value) override		\
	{																\
		//baseClass::Serialize(value);								\

#define END_DECLARE_SERIALIZER										\
	return FeEReturnCode::Success;									\
	}																\

#define DECLARE_PROPERTY_SERIALIZE(t,n)		/*FE_FAILEDRETURN( FeSerializerHelper::Serialize(value, &n, #n) );*/

#define DECLARE_SERIALIZER(properties, thisClass, baseClass)	\
	BEGIN_DECLARE_SERIALIZER(baseClass, thisClass)				\
	properties(DECLARE_PROPERTY_SERIALIZE)						\
	END_DECLARE_SERIALIZER										\

/// <summary>
/// Step 3 : Declare the function to deserialize class members from json
/// </summary>
#define BEGIN_DECLARE_DESERIALIZER(baseClass)						\
	virtual uint32 Deserialize(FeSerializerValue& value, uint32 iHeapId) override	\
	{																\
		baseClass::Deserialize(value, iHeapId);						\

#define END_DECLARE_DESERIALIZER									\
	return FeEReturnCode::Success;									\
	}																\

#define DECLARE_PROPERTY_DESERIALIZE(t,n)	FE_FAILEDRETURN( FeDeserialize(value, &n, #n, iHeapId) );

#define DECLARE_PROPERTY_ACCESSOR(t, n)				\
	const t & Get##n () const { return n ; }		\
	void Set##n (const t & value) { n = value; }	\

#define DECLARE_DESERIALIZER(properties, baseClass)	\
	BEGIN_DECLARE_DESERIALIZER(baseClass)			\
	properties(DECLARE_PROPERTY_DESERIALIZE)		\
	END_DECLARE_DESERIALIZER						\

#define DECLARE_ACCESSORS(properties, baseClass)	\
	properties(DECLARE_PROPERTY_ACCESSOR)			\

/// <summary>
/// Declares the properties of a class with serialization functions following the 3 steps defined above
/// </summary>
#define FE_DECLARE_CLASS_BODY(properties, thisClass, baseClass)	\
	private:													\
	DECLARE_CLASS_MEMBERS(properties)							\
	public:														\
	DECLARE_SERIALIZER(properties, thisClass, baseClass)		\
	DECLARE_DESERIALIZER(properties, baseClass)					\
	DECLARE_ACCESSORS(properties, baseClass)					\

#define FE_DECLARE_CLASS_DEFAULT_CTOR(thisClass, baseClass)		\
	public:														\
	thisClass() : baseClass() {}								\

#define FE_DECLARE_CLASS_BOTTOM(thisClass)																	\
	static FeTFactory<thisClass> Factory_##thisClass;														\
	static uint32 FeDeserialize(FeSerializerValue& value, thisClass* pOutput, const char* _sPropertyName, uint32 iHeapId)	\
	{																										\
		FeSerializerValue jsonProperty;																		\
																											\
		if (!FetchProperty(value, jsonProperty, _sPropertyName))											\
			return FeEReturnCode::Success;																	\
																											\
		return FeJsonParser::DeserializeObject(*pOutput, jsonProperty, iHeapId);							\
	}																										\


template<typename T>
struct FeTPtr
{
public:
	T* Ptr;

	FeTPtr() : Ptr(NULL) {}

	~FeTPtr()
	{
		Delete();
	}

	T* operator->() {
		return Ptr;
	}

	T* New()
	{
		Delete();
		Ptr = FE_NEW(T, 1);
	}

	void Delete()
	{
		if (Ptr)
		{
			FE_DELETE(T, Ptr, 1);
		}
	}
};

class FeSerializable
{
public:
	virtual uint32 Serialize(FeSerializerValue& serializer)
	{
		return FeEReturnCode::Success;
	}
	virtual uint32 Deserialize(FeSerializerValue& serializer, uint32 iHeapId)
	{
		return FeEReturnCode::Success;
	}
};

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

#define FE_DECLARE_ENUM(_name_, _values_)																			\
	namespace _name_																								\
	{																												\
		enum Type																									\
		{																											\
			_values_(FE_DECLARE_ENUM_VALUE)																			\
			Count																									\
		};																											\
	};																												\
	static uint32 FeDeserialize(FeSerializerValue& value, _name_::Type* pOutput, const char* _sPropertyName, uint32 iHeapId)\
	{																												\
		FeSerializerValue jsonProperty;																				\
		if (!FetchProperty(value, jsonProperty, _sPropertyName))													\
			return FeEReturnCode::Success;																			\
																													\
		static std::map<uint32, _name_::Type> iValuesMap;															\
		if (iValuesMap.size() == 0)																					\
		{																											\
			_values_(FE_DECLARE_ENUM_MAP_INJECTION)																	\
		}																											\
																													\
		uint32 iValue = FeStringTools::GenerateUIntIdFromString(jsonProperty.GetString());							\
																													\
		if (iValuesMap.find(iValue) != iValuesMap.end())															\
		{																											\
			*pOutput = iValuesMap[iValue];																			\
			return FeEReturnCode::Success;																			\
		}																											\
		else																										\
		{																											\
			FE_ASSERT(false, "Couldn't deserialize enum %s with value %s", #_name_, jsonProperty.GetString());		\
			return FeEReturnCode::Failed;																			\
		}																											\
	}																												\
FE_DECLARE_STRING_ENUM_MAP(_name_, _values_)																		\

