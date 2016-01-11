#include <moduleui.hpp>
#include <rendering/modulerenderer.hpp>
#include <rendering/modulerenderresourceshandler.hpp>

#include <common/jsonparser.hpp>

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
#define DECLARE_PROPERTIES(properties, thisClass, baseClass)	\
	private:													\
	DECLARE_CLASS_MEMBERS(properties)							\
	public:														\
	DECLARE_SERIALIZER(properties, thisClass, baseClass)		\
	DECLARE_DESERIALIZER(properties, baseClass)					\

template<typename T>
struct FePtr
{
public:
	T* Ptr;

	FePtr() : Ptr(NULL) {}
	~FePtr()
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
/*
	static uint32 Deserialize(FeSerializerValue& value, FeSerializable* pOutput, const char* _sPropertyName)
	{
		FeSerializerValue jsonProperty;

		if (!FetchProperty(value, jsonProperty, _sPropertyName))
			return FeEReturnCode::Success;

		return FeJsonParser::DeserializeObject(pOutput, jsonProperty);
	}*/
	
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
	static uint32 Deserialize(FeSerializerValue& value, FePtr<T>* pOutput, const char* _sPropertyName)
	{
		FeSerializerValue jsonProperty;

		if (!FetchProperty(value, jsonProperty, _sPropertyName))
			return FeEReturnCode::Success;

		if (jsonProperty.HasMember("_serialize_type_"))
		{
			pOutput->Ptr = (T*)CreateObjectPtrFromSerializedType(jsonProperty["_serialize_type_"].GetString());
		}

		return FeJsonParser::DeserializeObject(*pOutput->Ptr, jsonProperty);
	}
	
	static uint32 Deserialize(FeSerializerValue& value, int* pOutput, const char* _sPropertyName)
	{
		FeSerializerValue jsonProperty;

		if (FetchProperty(value, jsonProperty, _sPropertyName))
			*pOutput = jsonProperty.GetInt();

		return FeEReturnCode::Success;
	}
	
	static uint32 Deserialize(FeSerializerValue& value, FePath* pOutput, const char* _sPropertyName)
	{
		FeSerializerValue jsonProperty;

		if (FetchProperty(value, jsonProperty, _sPropertyName))
			pOutput->Set(jsonProperty.GetString());

		return FeEReturnCode::Success;
	}

	static uint32 Deserialize(FeSerializerValue& value, FeTransform* pOutput, const char* _sPropertyName)
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

	static void* CreateObjectPtrFromSerializedType(const char*);
};
class FeTestObjectBase : public FeSerializable
{
public:
	#define FeTestObjectBase_Properties(_d)		\
		_d(int,				Count)				\
		_d(FeTArray<int>,	Values)				\
		_d(FeTransform,		Transform)			\

	DECLARE_PROPERTIES(FeTestObjectBase_Properties, FeTestObjectBase, FeSerializable)
};

class FeTestObjectChild : public FeTestObjectBase
{
public:
	#define FeTestObjectChild_Properties(_d)	\
		_d(FePath,	File)						\

	DECLARE_PROPERTIES(FeTestObjectChild_Properties, FeTestObjectChild, FeTestObjectBase)
};


class FeTestObjectA : public FeSerializable
{
	#define FeTestObjectA_Properties(_d)			\
		_d(FeTArray<FePtr<FeTestObjectBase>>,Objs)	\

	DECLARE_PROPERTIES(FeTestObjectA_Properties, FeTestObjectA, FeSerializable)
};

void* FeSerializerHelper::CreateObjectPtrFromSerializedType(const char* _sType)
{
	std::string sType = _sType;

	void* pOutput = NULL;

	if (sType == "FeTestObjectChild")
		pOutput = FE_NEW(FeTestObjectChild, 1);

	return pOutput;
}


uint32 FeModuleUi::Load(const FeModuleInit* initBase)
{
	auto init = (FeModuleUiInit*)initBase;
	srand(1564);

	{
		FeTestObjectA obj;
		auto iRes = FeJsonParser::DeserializeObject(obj, "../data/test/ui/component.json");
	}
	FeTArray<FePath> files;
	files.SetHeapId(RENDERER_HEAP);

	FeFileTools::ListFilesRecursive("../data/test/textures/boxfronts", "*.jpg", files);
	FeFileTools::ListFilesRecursive("../data/test/textures/bb2VFX", "*", files);
	uint32 iTexturesCount = files.GetSize();
	//iTexturesCount = 5;

	// Creat textures
	auto pResourcesHandler = FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();
	GeometryInstances.SetHeapId(RENDERER_HEAP);
	const int InstancesCount = 1024*1;
		
	GeometryInstances.Reserve(InstancesCount);

	for (uint32 i = 0; i < InstancesCount; ++i)
	{
		 FeRenderGeometryInstance& geomInstance = GeometryInstances.Add();

		geomInstance.Effect = RENDERER_DEFAULT_EFFECT_ID;
		geomInstance.Geometry = FeGeometryHelper::GetStaticGeometry(FeEGemetryDataType::Quad);
		
		if (iTexturesCount>0)
		{
			// bind texture
			FeRenderTextureId textureId;
			uint32 iTexIdx = i % iTexturesCount;
			pResourcesHandler->LoadTexture(files[iTexIdx].Value, &textureId);
			geomInstance.Textures.Add(textureId);
		}
	}
		
	return FeEReturnCode::Success;
}
uint32 FeModuleUi::Unload()
{
	return FeEReturnCode::Success;
}
uint32 FeModuleUi::Update(const FeDt& fDt)
{
	static float fRotX = 0, fRotY = 0, fRotZ = 0;
	static FeVector3 translation(0, 0, 0), scale(1, 1, 1);
	static float fIncrement = 0.0f;
		
	int32 iColomns = 60;
	fIncrement += 0.15f*fDt.TotalSeconds;

	float fOffset = (1.01f-abs(sin(fIncrement)))*3.0f;

	float fOffsetBetweenX = 1.f*fOffset;
	float fOffsetBetweenY = 1.0f*fOffset;

	translation.mData[0] = (float)-iColomns;
	translation.mData[1] = 0;
	translation.mData[2] = 50.0f;// -fOffset;

	scale.mData[0] = 1.0f + fOffset;
	scale.mData[1] = 1.0f + fOffset;

	fRotZ = fOffset;

	auto pRenderingModule = FeApplication::StaticInstance.GetModule<FeModuleRendering>();

	FeRenderBatch& renderBatch = pRenderingModule->CreateRenderBatch();
	renderBatch.GeometryInstances.Reserve(GeometryInstances.GetSize());

	for (uint32 iInstanceIdx = 0; iInstanceIdx < GeometryInstances.GetSize(); ++iInstanceIdx)
	{
		FeRenderGeometryInstance& geomInstance = GeometryInstances[iInstanceIdx];

		translation.mData[0] = -40 + (iInstanceIdx % iColomns) * fOffsetBetweenX;
		translation.mData[1] = -20 + (iInstanceIdx / iColomns) * fOffsetBetweenY;

		FeGeometryHelper::ComputeAffineTransform(geomInstance.Transform, translation, FeRotation(fRotX, fRotY, fRotZ), scale);

		renderBatch.GeometryInstances.Add(geomInstance);
	}
	return FeEReturnCode::Success;
}
