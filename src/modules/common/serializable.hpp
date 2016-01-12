#pragma once 

#include <common.hpp>
#include <jsonparser.hpp>

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
				{															\
		//baseClass::Serialize(value);								\

#define END_DECLARE_SERIALIZER										\
	return FeEReturnCode::Success;									\
				}															\

#define DECLARE_PROPERTY_SERIALIZE(t,n)		/*FE_FAILEDRETURN( FeSerializerHelper::Serialize(value, &n, #n) );*/

#define DECLARE_SERIALIZER(properties, thisClass, baseClass)	\
	BEGIN_DECLARE_SERIALIZER(baseClass, thisClass)				\
	properties(DECLARE_PROPERTY_SERIALIZE)						\
	END_DECLARE_SERIALIZER										\

/// <summary>
/// Step 3 : Declare the function to deserialize class members from json
/// </summary>
#define BEGIN_DECLARE_DESERIALIZER(baseClass)						\
	virtual uint32 Deserialize(FeSerializerValue& value) override	\
				{															\
		baseClass::Deserialize(value);								\

#define END_DECLARE_DESERIALIZER									\
	return FeEReturnCode::Success;									\
				}															\

#define DECLARE_PROPERTY_DESERIALIZE(t,n)	FE_FAILEDRETURN( FeSerializerHelper::Deserialize(value, &n, #n) );

#define DECLARE_DESERIALIZER(properties, baseClass)	\
	BEGIN_DECLARE_DESERIALIZER(baseClass)			\
	properties(DECLARE_PROPERTY_DESERIALIZE)		\
	END_DECLARE_DESERIALIZER						\

/// <summary>
/// Declares the properties of a class with serialization functions following the 3 steps defined above
/// </summary>
#define FE_DECLARE_CLASS_BODY(properties, thisClass, baseClass)	\
	private:													\
	/*static const FeTFactory<thisClass> Factory;*/					\
	/*const TestObj obj;*/											\
	DECLARE_CLASS_MEMBERS(properties)							\
	public:														\
	DECLARE_SERIALIZER(properties, thisClass, baseClass)		\
	DECLARE_DESERIALIZER(properties, baseClass)					\
	private:													\
	//const thisClass __const_instance__;						\

#define FE_DECLARE_CLASS_DEFAULT_CTOR(thisClass, baseClass)		\
	public:														\
	thisClass() : baseClass() {}								\

#define FE_DECLARE_CLASS_BOTTOM(thisClass)						\
	static FeTFactory<thisClass> Factory_##thisClass;			\


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
	virtual uint32 Deserialize(FeSerializerValue& serializer)
	{
		return FeEReturnCode::Success;
	}
};

#include "maths.hpp"

class FeSerializerHelper
{
private:
	static bool FetchProperty(FeSerializerValue& obj, FeSerializerValue& property, const char* sPropertyName)
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
public:

	static uint32 Serialize(FeSerializerValue& value, const void* pInput, const char* _sPropertyName)
	{
		return FeEReturnCode::Success;
	}

	template<typename T>
	static uint32 Deserialize(FeSerializerValue& value, FeTArray<T>* pOutput, const char* _sPropertyName)
	{
		FeSerializerValue jsonProperty;

		if (!FetchProperty(value, jsonProperty, _sPropertyName))
			return FeEReturnCode::Success;

		if (jsonProperty.IsArray())
		{
			size_t iSize = jsonProperty.Size();
			pOutput->Clear();
			pOutput->Reserve(iSize);

			for (size_t i = 0; i < iSize; ++i)
			{
				FeSerializerValue& element = jsonProperty[i];
				T& elementObj = pOutput->Add();

				Deserialize(element, &elementObj, ".");
			}
		}
		return FeEReturnCode::Success;
	}

	template<typename T>
	static uint32 Deserialize(FeSerializerValue& value, FeTPtr<T>* pOutput, const char* _sPropertyName)
	{
		FeSerializerValue jsonProperty;

		if (!FetchProperty(value, jsonProperty, _sPropertyName))
			return FeEReturnCode::Success;

		if (jsonProperty.HasMember("_serialize_type_"))
		{
			pOutput->Ptr = (T*)GetObjectsFactory().CreateObjectFromFactory(jsonProperty["_serialize_type_"].GetString());
		}

		return FeJsonParser::DeserializeObject(*pOutput->Ptr, jsonProperty);
	}

	static uint32 Deserialize(FeSerializerValue& value, int*			pOutput,	const char* _sPropertyName);
	static uint32 Deserialize(FeSerializerValue& value, float*			pOutput,	const char* _sPropertyName);
	static uint32 Deserialize(FeSerializerValue& value, uint32*			pOutput,	const char* _sPropertyName);
	static uint32 Deserialize(FeSerializerValue& value, FePath*			pOutput,	const char* _sPropertyName);
	static uint32 Deserialize(FeSerializerValue& value, FeVector3*		pOutput, const char* _sPropertyName);
	static uint32 Deserialize(FeSerializerValue& value, FeColor*		pOutput, const char* _sPropertyName);
	static uint32 Deserialize(FeSerializerValue& value, FeTransform*	pOutput,	const char* _sPropertyName);
};
