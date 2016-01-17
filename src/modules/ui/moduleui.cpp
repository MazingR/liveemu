#include <moduleui.hpp>
#include <rendering/modulerenderer.hpp>
#include <rendering/renderresourceshandler.hpp>

#include <common/serializable.hpp>
#include <common/maths.hpp>
#include <queue>


void OnScriptFileChanged(FeEFileChangeType::Type eChangeType, const char* szPath, void* pUserData)
{
	((FeModuleUi*)pUserData)->ReloadScripts();
}

uint32 FeModuleUi::LoadUnitTest(uint32 iTest)
{
	/*
	FeTArray<FePath> files;
	files.SetHeapId(RENDERER_HEAP);

	FeFileTools::ListFilesRecursive(files, "../data/test/textures/boxfronts", "*.jpg");
	FeFileTools::ListFilesRecursive(files, "../data/test/textures/bb2VFX", "*");
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
	*/

	return FeEReturnCode::Success;
}
uint32 FeModuleUi::UpdateUnitTest(uint32 iTest, const FeDt& fDt)
{
	/*
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
	*/
	return FeEReturnCode::Success;
}
uint32 FeModuleUi::ReloadScripts()
{
	auto pRenderingModule = FeApplication::StaticInstance.GetModule<FeModuleRendering>();

	// Clear stuff
	pRenderingModule->UnloadEffects();
	ScriptFiles.Clear();
	RenderingInstances.Clear();
	Panels.Clear();

	// Load scripts from files
	FeTArray<FePath> files;
	files.SetHeapId(RENDERER_HEAP);

	FeFileTools::ListFilesRecursive(files, "themes/common", ".*\\.fes");
	FeFileTools::ListFilesRecursive(files, "themes/default", ".*\\.fes"); // load default theme

	for (auto& file : files)
	{
		FeScriptFile& scriptFile = ScriptFiles.Add();
		auto iRes = FeJsonParser::DeserializeObject(scriptFile, file);

		if (iRes == FeEReturnCode::Success)
		{
			pRenderingModule->LoadEffects(scriptFile.GetEffects());

			for (auto& panel : scriptFile.GetPanels())
			{
				Panels.Add(&panel);
			}
		}
		else
		{
			ScriptFiles.PopBack();
		}
	}

	// Pre compute rendering instances
	std::queue<FeUiElement*> uiElements;

	for (auto& panel : Panels)
		uiElements.push(panel);

	while (uiElements.size())
	{
		FeUiElement* pElement = uiElements.front();
		uiElements.pop();

		FeUiRenderingInstance& renderingInstance = RenderingInstances.Add();

		renderingInstance.Owner = pElement;
		renderingInstance.Geometry.Effect = pElement->GetEffect().Id();
		renderingInstance.Geometry.Geometry = FeGeometryHelper::GetStaticGeometry(FeEGemetryDataType::Quad);

		FeGeometryHelper::ComputeAffineTransform(renderingInstance.Geometry.Transform,
			pElement->GetTransform().Translation,
			pElement->GetTransform().Rotation,
			pElement->GetTransform().Scale);


		for (auto& child : pElement->GetChildren())
			uiElements.push(child.Ptr);
	}

	return FeEReturnCode::Success;
}
uint32 FeModuleUi::Load(const FeModuleInit* initBase)
{
	auto init = (FeModuleUiInit*)initBase;
	ReloadScripts();
	//LoadUnitTest(0);

	auto pFileManagerModule = FeApplication::StaticInstance.GetModule<FeModuleFilesManager>();
	pFileManagerModule->WatchDirectory("../data/themes", OnScriptFileChanged, this);
		
	return FeEReturnCode::Success;
}
uint32 FeModuleUi::Unload()
{
	return FeEReturnCode::Success;
}
uint32 FeModuleUi::Update(const FeDt& fDt)
{
	//UpdateUnitTest(0, fDt);

	auto pRenderingModule = FeApplication::StaticInstance.GetModule<FeModuleRendering>();

	FeRenderBatch& renderBatch = pRenderingModule->CreateRenderBatch();
	renderBatch.GeometryInstances.Reserve(RenderingInstances.GetSize());

	for (auto& instance : RenderingInstances)
	{
		renderBatch.GeometryInstances.Add(instance.Geometry);
	}

	return FeEReturnCode::Success;
}
