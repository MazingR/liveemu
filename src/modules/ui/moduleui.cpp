#include <moduleui.hpp>
#include <rendering/modulerenderer.hpp>
#include <rendering/renderresourceshandler.hpp>

#include <common/serializable.hpp>
#include <common/maths.hpp>

#define FeEUiElementState_Values(_d)		\
		_d(FeEUiElementState, Visible)		\
		_d(FeEUiElementState, Collapsed)	\
		_d(FeEUiElementState, Selected)		\

FE_DECLARE_ENUM(FeEUiElementState, FeEUiElementState_Values)

class FeUiRenderEffect : public FeSerializable
{
public:
	bool HasState(FeEUiElementState::Type state);
	bool IsVisible();
	bool IsSelected();

#define FeUiRenderEffect_Properties(_d)					\
		_d(FeString,							Name)	\

	FE_DECLARE_CLASS_BODY(FeUiRenderEffect_Properties, FeUiRenderEffect, FeSerializable)
};
FE_DECLARE_CLASS_BOTTOM(FeUiRenderEffect)

class FeUiElement : public FeSerializable
{
public:
	bool HasState(FeEUiElementState::Type state);
	bool IsVisible();
	bool IsSelected();

	#define FeUiElement_Properties(_d)					\
		_d(FeTransform,						Transform)	\
		_d(FeEUiElementState::Type,			State)		\
		_d(FeTArray<FeTPtr<FeUiElement>>,	Children)

	FE_DECLARE_CLASS_BODY(FeUiElement_Properties, FeUiElement, FeSerializable)
};
FE_DECLARE_CLASS_BOTTOM(FeUiElement)


bool FeUiElement::HasState(FeEUiElementState::Type state)
{
	return (this->State & state) != 0;
}
bool FeUiElement::IsSelected()
{
	return HasState(FeEUiElementState::Selected);
}
bool FeUiElement::IsVisible()
{
	return HasState(FeEUiElementState::Visible);
}

class FeUiPanel : public FeUiElement
{
public:
#define FeUiPanel_Properties(_d)		\
	_d(FeTArray<FeUiElement>, Children)		\

	FE_DECLARE_CLASS_BODY(FeUiPanel_Properties, FeUiPanel, FeUiElement)
};
FE_DECLARE_CLASS_BOTTOM(FeUiPanel)


class FeScriptFile : public FeUiElement
{
public:
#define FeScriptFile_Properties(_d)					\
	_d(FeTArray<FeUiPanel>,			UiPanels)		\
	_d(FeTArray<FeUiRenderEffect>,	UiEffects)		\

	FE_DECLARE_CLASS_BODY(FeScriptFile_Properties, FeUiElement, FeSerializable)
};
FE_DECLARE_CLASS_BOTTOM(FeScriptFile)

uint32 FeModuleUi::LoadUnitTest(uint32 iTest)
{
	FeTArray<FePath> files;
	files.SetHeapId(RENDERER_HEAP);

	FeFileTools::ListFilesRecursive("../data/test/textures/boxfronts", "*.jpg", files);
	FeFileTools::ListFilesRecursive("../data/test/textures/bb2VFX", "*", files);
	uint32 iTexturesCount = files.GetSize();
	//iTexturesCount = 5;

	// Creat textures
	auto pResourcesHandler = FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();
	GeometryInstances.SetHeapId(RENDERER_HEAP);
	const int InstancesCount = 1024 * 1;

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
uint32 FeModuleUi::UpdateUnitTest(uint32 iTest, const FeDt& fDt)
{
	srand(1564);

	static float fRotX = 0, fRotY = 0, fRotZ = 0;
	static FeVector3 translation(0, 0, 0), scale(1, 1, 1);
	static float fIncrement = 0.0f;

	int32 iColomns = 60;
	fIncrement += 0.15f*fDt.TotalSeconds;

	float fOffset = (1.01f - abs(sin(fIncrement)))*3.0f;

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

uint32 FeModuleUi::Load(const FeModuleInit* initBase)
{
	auto init = (FeModuleUiInit*)initBase;
	
	{
		FeScriptFile obj;
		auto iRes = FeJsonParser::DeserializeObject(obj, "../data/test/ui/script.json");
	}

	//LoadUnitTest(0);
		
	return FeEReturnCode::Success;
}
uint32 FeModuleUi::Unload()
{
	return FeEReturnCode::Success;
}
uint32 FeModuleUi::Update(const FeDt& fDt)
{
	//UpdateUnitTest(0, fDt);
	return FeEReturnCode::Success;
}
