#include <moduleui.hpp>
#include <rendering/modulerenderer.hpp>
#include <rendering/renderresourceshandler.hpp>

#include <common/serializable.hpp>
#include <common/maths.hpp>
#include <queue>

uint32 FeModuleUi::LoadUnitTest(uint32 iTest)
{
	FeTArray<FePath> files;
	files.SetHeapId(RENDERER_HEAP);

	FeFileTools::ListFilesRecursive(files, "../data/test/textures/boxfronts", "*.jpg");
	FeFileTools::ListFilesRecursive(files, "../data/test/textures/bb2VFX", "*");
	uint32 iTexturesCount = files.GetSize();
	//iTexturesCount = 5;

	// Creat textures
	auto pResourcesHandler = FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();
	
	const int InstancesCount = 1;
	FeString szEffect = FeStringPool::GetInstance()->CreateString("Default");

	for (uint32 i = 0; i < InstancesCount; ++i)
	{
		auto pPanel = FE_NEW(FeUiPanel, 1);
		Panels.Add(pPanel);

		pPanel->SetEffect(szEffect);

		if (iTexturesCount>0)
		{
			FeRenderLoadingResource resLoading;
			resLoading.Type = FeEResourceType::Texture;
			uint32 iTexIdx = i % iTexturesCount;
			resLoading.Path = files[iTexIdx];
			
			pResourcesHandler->LoadResource(resLoading); 
		}
	}

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
