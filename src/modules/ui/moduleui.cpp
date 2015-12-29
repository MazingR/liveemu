#include <moduleui.hpp>
#include <rendering/modulerenderer.hpp>
#include <rendering/modulerenderresourceshandler.hpp>

uint32 FeModuleUi::Load(const FeModuleInit* initBase)
{
	auto init = (FeModuleUiInit*)initBase;
	srand(1564);

	FeTArray<FeFile> files;
	FeFileSystem::ListFilesRecursive("../data/test/textures/boxfronts", "*", files);
	uint32 iTexturesCount = files.GetSize();
	//iTexturesCount = 5;

	// Creat textures
	auto pResourcesHandler = FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();
	GeometryInstances.SetHeapId(RENDERER_HEAP);
	const int InstancesCount = 1024*16;
		
	GeometryInstances.Reserve(InstancesCount);

	for (uint32 i = 0; i < InstancesCount; ++i)
	{
		FeRenderGeometryInstance* pGeomInstance = FE_NEW(FeRenderGeometryInstance, RENDERER_HEAP);
		GeometryInstances.Add(pGeomInstance);

		pGeomInstance->Effect = RENDERER_DEFAULT_EFFECT_ID;
		pGeomInstance->Geometry = FeGeometryHelper::GetStaticGeometry(FeEGemetryDataType::Quad);
		
		if (iTexturesCount>0)
		{
			// bind texture
			FeRenderTextureId textureId;
			uint32 iTexIdx = rand() % iTexturesCount;
			pResourcesHandler->LoadTexture(files[iTexIdx].Path, &textureId);
			pGeomInstance->Textures.Add(textureId);
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

	const int iBatchMaxSize = 2048;
	uint32 iBatchesCount = (uint32)ceil(GeometryInstances.GetSize() / (float)iBatchMaxSize);
	FeTArray<FeRenderBatch*> batches;

	for (uint32 i = 0; i < iBatchesCount; ++i)
	{
		FeRenderBatch& renderBatch = pRenderingModule->CreateRenderBatch();
		renderBatch.GeometryInstances.Reserve(2048);
		batches.Add(&renderBatch);
	}

	for (uint32 iInstanceIdx = 0; iInstanceIdx < GeometryInstances.GetSize(); ++iInstanceIdx)
	{
		FeRenderGeometryInstance& geomInstance = *GeometryInstances[iInstanceIdx];

		translation.mData[0] = -40 + (iInstanceIdx % iColomns) * fOffsetBetweenX;
		translation.mData[1] = -20 + (iInstanceIdx / iColomns) * fOffsetBetweenY;

		FeGeometryHelper::ComputeAffineTransform(geomInstance.Transform, translation, FeRotation(fRotX, fRotY, fRotZ), scale);

		batches[iInstanceIdx/iBatchMaxSize]->GeometryInstances.Add(geomInstance);
	}
	return FeEReturnCode::Success;
}
