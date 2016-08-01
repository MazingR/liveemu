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
	virtual uint32 Serialize(FeSerializerValue& valueSerialize, FeESerializeFormat::Type format = FeESerializeFormat::Json) override		\
	{																\
		//baseClass::Serialize(value, format);						\

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
	virtual uint32 Deserialize(FeSerializerValue& value, uint32 iHeapId, FeESerializeFormat::Type format = FeESerializeFormat::Json) override	\
	{																\
		baseClass::Deserialize(value, iHeapId, format);						\

#define END_DECLARE_DESERIALIZER									\
	return FeEReturnCode::Success;									\
	}																\

#define DECLARE_PROPERTY_DESERIALIZE(t,n)																\
switch (format)																							\
{																										\
case FeESerializeFormat::Json: FE_FAILEDRETURN(FeDeserialize(value, &n, #n, iHeapId)); break;			\
default:																								\
	FE_ASSERT(false, "Uknwon serialize format");														\
}																										\

/// <summary>
/// Step 4 : Declare the function to de/serialize class members from/to sql script command
/// </summary>

template<typename T>
void ComputeSqlPropertyName(const T& value, const char* sTypeName, const char* sName, char* output, uint32 iOutputLen, bool bInsert, uint32& iOutputedLen)
{
	if (!bInsert || strcmp(sName, "ID") != 0)
	{ 
		iOutputedLen += sprintf_s(output + iOutputedLen, iOutputLen - iOutputedLen, "'%s', ", sName);
	}
}
void ComputeSqlPropertyName(const FeTPtr<FeSerializable>* pValue, const char* sTypeName, const char* sName, char* output, uint32 iOutputLen, bool bInsert, uint32& iOutputedLen);

template<typename T>
void ComputeSqlRefPropertyInsert(const T& value, const char* sTypeName, const char* sName, char* output, uint32 iOutputLen, bool bInsert, uint32& iOutputedLen)
{}
void ComputeSqlRefPropertyInsert(const FeTPtr<FeSerializable>* pValue, const char* sTypeName, const char* sName, char* output, uint32 iOutputLen, bool bInsert, uint32& iOutputedLen);

template<typename T>
void ComputeSqlRefPropertyLink(const T& value, const char* sTypeName, const char* sName, char* output, uint32 iOutputLen, bool bInsert, uint32& iOutputedLen)
{}
void ComputeSqlRefPropertyLink(const FeTPtr<FeSerializable>* pValue, const char* sTypeName, const char* sName, char* output, uint32 iOutputLen, bool bInsert, uint32& iOutputedLen);


template<typename T>
void ComputeSqlPropertyValue(const T& value, const char* sTypeName, const char* sName, char* output, uint32 iOutputLen, bool bInsert, uint32& iOutputedLen)
{
	if (!bInsert || strcmp(sName, "ID") != 0)
	{
		iOutputedLen += sprintf_s(output + iOutputedLen, iOutputLen - iOutputedLen, "'");
		iOutputedLen += FeSeserializeDb(output + iOutputedLen, iOutputLen - iOutputedLen, value);
		iOutputedLen += sprintf_s(output + iOutputedLen, iOutputLen - iOutputedLen, "', ");
	}
}
void ComputeSqlPropertyValue(const FeTPtr<FeSerializable>* pValue, const char* sTypeName, const char* sName, char* output, uint32 iOutputLen, bool bInsert, uint32& iOutputedLen);

#define DECLARE_SQL_SERIALIZE_PROPERTY_NAME(t,n)			ComputeSqlPropertyName(n, #t, #n, output, iOutputLen, bInsert, iOutputedLen);
#define DECLARE_SQL_SERIALIZE_PROPERTY(t,n)					ComputeSqlPropertyValue(n, #t, #n, output, iOutputLen, bInsert, iOutputedLen);
#define DECLARE_SQL_SERIALIZE_REF_PROPERTY_INSERT(t,n)		ComputeSqlRefPropertyInsert(n, #t, #n, output, iOutputLen, bInsert, iOutputedLen);
#define DECLARE_SQL_SERIALIZE_REF_PROPERTY_LINK(t,n)		ComputeSqlRefPropertyLink(n, #t, #n, output, iOutputLen, bInsert, iOutputedLen);

#define DECLARE_SQL_SERIALIZER(properties, thisClass, baseClass)																\
	virtual void ComputeSqlPropertiesName(char* output, uint32 iOutputLen, uint32& iOutputedLen, bool bInsert) override			\
	{																															\
		baseClass::ComputeSqlPropertiesName(output, iOutputLen, iOutputedLen, bInsert);											\
		properties(DECLARE_SQL_SERIALIZE_PROPERTY_NAME);																		\
	}																															\
	virtual void ComputeSqlPropertiesValues(char* output, uint32 iOutputLen, uint32& iOutputedLen, bool bInsert) override		\
	{																															\
		baseClass::ComputeSqlPropertiesValues(output, iOutputLen, iOutputedLen, bInsert);										\
		properties(DECLARE_SQL_SERIALIZE_PROPERTY);																				\
		properties(DECLARE_SQL_SERIALIZE_REF_PROPERTY_LINK);																	\
	}																															\
	void ComputeSqlInsertOrUpdate(char* output, uint32 iOutputLen, uint32& iOutputedLen, bool bInsert)							\
	{																															\
		properties(DECLARE_SQL_SERIALIZE_REF_PROPERTY_INSERT);																	\
		iOutputedLen = 0;																										\
		iOutputedLen += sprintf_s(output + iOutputedLen, iOutputLen - iOutputedLen, "INSERT OR REPLACE INTO %s(", #thisClass);	\
		ComputeSqlPropertiesName(output, iOutputLen, iOutputedLen, bInsert);													\
		iOutputedLen -= 2; /*remove 2 chars ", " */																				\
		iOutputedLen += sprintf_s(output + iOutputedLen, iOutputLen - iOutputedLen, ") VALUES(");								\
		ComputeSqlPropertiesValues(output, iOutputLen, iOutputedLen, bInsert);													\
		iOutputedLen -= 2;; /*remove 2 chars ", " */																			\
		iOutputedLen += sprintf_s(output + iOutputedLen, iOutputLen - iOutputedLen, ")");										\
	}																															\
	void ComputeSqlUpdate(char* output, uint32 iOutputLen, uint32& iOutputedLen)												\
	{																															\
		ComputeSqlInsertOrUpdate(output, iOutputLen, iOutputedLen, false);														\
	}																															\
	void ComputeSqlInsert(char* output, uint32 iOutputLen, uint32& iOutputedLen)												\
	{																															\
		ComputeSqlInsertOrUpdate(output, iOutputLen, iOutputedLen, true);														\
	}

/// <summary>
/// Step 5 : Declare the properties reflection acces function
/// </summary>
/// 
#define DECLARE_REFLECTION_PROPERTY(t, n) if (strcmp(szName, #n) == 0) return &n;

#define DECLARE_REFLECTION(properties, thisClass, baseClass)																\
	virtual void* GetPropertyValueByName(const char* szName) override														\
	{																														\
		void* Value = baseClass::GetPropertyValueByName(szName);															\
		if (!Value)																											\
		{																													\
			properties(DECLARE_REFLECTION_PROPERTY)																			\
		}																													\
		return Value;																										\
	}																														\

// ------------------------------------------------------------------------------------------------

#define DECLARE_PROPERTY_ACCESSOR(t, n)				\
	const t & Get##n () const { return n ; }		\
	t & Get##n () { return n ; }					\
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
	public:														\
	static const char* ClassName() { return #thisClass; }		\
	protected:													\
	DECLARE_CLASS_MEMBERS(properties)							\
	public:														\
	DECLARE_SERIALIZER(properties, thisClass, baseClass)		\
	DECLARE_DESERIALIZER(properties, baseClass)					\
	DECLARE_SQL_SERIALIZER(properties, thisClass, baseClass)	\
	DECLARE_ACCESSORS(properties, baseClass)					\
	DECLARE_REFLECTION(properties, thisClass, baseClass)		\

#define FE_DECLARE_CLASS_BOTTOM(thisClass)																					\
	static FeTFactory<thisClass> Factory_##thisClass;																		\
	static uint32 FeDeserialize(FeSerializerValue& value, thisClass* pOutput, const char* _sPropertyName, uint32 iHeapId)	\
	{																										\
		FeSerializerValue jsonProperty;																		\
																											\
		if (!FetchProperty(value, jsonProperty, _sPropertyName))											\
			return FeEReturnCode::Success;																	\
																											\
		return FeJsonParser::DeserializeObject(*pOutput, jsonProperty, iHeapId);							\
	}																										\
	static uint32 FeDeserializeDb(FeSerializerValue& value, thisClass* pOutput, const char* _sPropertyName, uint32 iHeapId)	\
		{																									\
		return FeEReturnCode::Success;																		\
		}																									\

