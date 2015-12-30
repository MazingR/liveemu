#include <modulerenderer.hpp>
#include <modulerenderresourceshandler.hpp>
#include <common/memorymanager.hpp>

#include <d3dx11include.hpp>
#include "FW1FontWrapper.h"
#include <SDL.h>

#define UPDATE_RENDER_INFOS_FREQUENCY 30
#define D3DFAILEDRETURN(func) { HRESULT ___hr = (func); if (___hr!=S_OK) return ___hr; }

uint32 FeModuleRendering::Load(const FeModuleInit* initBase)
{
	RegisteredRenderBatches.SetHeapId(RENDERER_HEAP);

	auto init = (FeModuleRenderingInit*)initBase;

	FE_FAILEDRETURN(Device.Initialize(init->WindowHandle));

	FeRenderEffect& newEffect = Effects.Add();
	FE_FAILEDRETURN(newEffect.CreateFromFile("../data/themes/common/shaders/default.fx"));

	// Creat static geometries (primitive forms)
	Geometries.Reserve(16);
	Geometries.SetZeroMemory();
	FeRenderGeometryData& geometryData = Geometries.Add();
	FeRenderGeometryId geometryId;
	FeGeometryHelper::CreateStaticGeometry(FeEGemetryDataType::Quad, &geometryData, &geometryId);

	HRESULT hResult = FW1CreateFactory(FW1_VERSION, &FW1Factory);
	hResult = FW1Factory->CreateFontWrapper(Device.GetD3DDevice(), L"Impact", &FontWrapper);

	CurrentDebugTextMode = FeEDebugRenderTextMode::Rendering;

	ZeroMemory(&RenderDebugInfos, sizeof(FeRenderDebugInfos));

	DefaultViewport.CreateFromBackBuffer();

	return FeEReturnCode::Success;
}
uint32 FeModuleRendering::Unload()
{
	Device.Release();

	for (uint32 i = 0; i < Effects.GetSize(); ++i)
		Effects[i].Release();

	FeGeometryHelper::ReleaseGeometryData();
	FeGeometryHelper::ReleaseStaticGeometryData();

	SafeRelease(FontWrapper);
	SafeRelease(FW1Factory);

	return FeEReturnCode::Success;
}
uint32 FeModuleRendering::Update(const FeDt& fDt)
{
	int iProcessedMsg = 0;
	int iMaxProcessedMsg = 3;

	MSG msg = { 0 };

	// Process iMaxProcessedMsg at maximum
	//while (WM_QUIT != msg.message && iProcessedMsg++<iMaxProcessedMsg)
	//{
	//	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	//	{
	//		TranslateMessage(&msg);
	//		DispatchMessage(&msg);
	//	}
	//}

	// Render frame
	BeginRender();
	{
		// Render all registered batches
		for (uint32 i = 0; i < RegisteredRenderBatches.GetSize(); ++i)
		{
			RenderBatch(RegisteredRenderBatches[i], fDt);
			RegisteredRenderBatches[i].GeometryInstances.Clear();
		}
		RenderDebugText(fDt);
	}
	EndRender();

	if (WM_QUIT == msg.message)
	{
		return FeEReturnCode::Canceled;
	}

	return FeEReturnCode::Success;
}
void FeModuleRendering::BeginRender()
{
	RenderDebugInfos.FrameDrawCallsCount = 0;
	RenderDebugInfos.FrameBindEffectCount = 0;
	RenderDebugInfos.FrameBindGeometryCount = 0;
	RenderDebugInfos.FrameBindTextureCount = 0;

	for (uint32 i = 0; i < RegisteredRenderBatches.GetSize(); ++i)
		RegisteredRenderBatches[i].Viewport->Clear();
		
}
void FeModuleRendering::EndRender()
{
	uint32 iTicks = SDL_GetTicks();
	// Present the information rendered to the back buffer to the front buffer (the screen)
	Device.GetSwapChain()->Present(0, 0);
		
	RegisteredRenderBatches.Clear();
}
void FeModuleRendering::RenderBatch(FeRenderBatch& batch, const FeDt& fDt)
{
	// Clear the back buffer
	batch.Viewport->Bind();

	FeTArray<FeRenderGeometryInstance>& instances = batch.GeometryInstances;

	FeRenderEffectId iLastEffectId = 0;
	FeRenderGeometryId iLastGeometryId = 0;
		
	FeRenderGeometryData* pGeometryData = NULL;
	FeRenderEffect* pEffect = NULL;
	FeRenderCamera camera;

	auto pResourcesHandler = FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();
	ID3D11DeviceContext* pContext = GetDevice().GetImmediateContext();

	FeRenderTextureId lastBindedTextures[16];
	ZeroMemory(lastBindedTextures, sizeof(FeRenderTextureId) * 16);

	for (uint32 i = 0; i < Effects.GetSize() ; ++i)
	{
		Effects[i].BeginFrame(camera, *batch.Viewport);
	}

	for (uint32 iInstanceIdx = 0; iInstanceIdx < instances.GetSize(); ++iInstanceIdx)
	{
		FeRenderGeometryInstance& geomInstance = instances[iInstanceIdx];

		if (geomInstance.Effect != iLastEffectId)
		{
			FE_ASSERT(Effects.GetSize() >= geomInstance.Effect, "Invalid effect Id !");
			pEffect = &Effects[geomInstance.Effect - 1];
			pEffect->Bind();
			iLastEffectId = geomInstance.Effect;

			RenderDebugInfos.FrameBindEffectCount++;
		}

		pEffect->BindGeometryInstance(geomInstance, pResourcesHandler);

		// Set resources (textures)
		for (uint32 iTextureIdx = 0; iTextureIdx < geomInstance.Textures.GetSize(); ++iTextureIdx)
		{
			const FeRenderTextureId& textureId = geomInstance.Textures[iTextureIdx];
			if (lastBindedTextures[iTextureIdx] != textureId)
			{
				lastBindedTextures[iTextureIdx] = textureId;
				const FeRenderTexture* pTexture = pResourcesHandler->GetTexture(textureId);

				if (pTexture && pTexture->LoadingState == FeETextureLoadingState::Loaded)
				{
					pContext->PSSetShaderResources(iTextureIdx, 1, &pTexture->SRV);
					RenderDebugInfos.FrameBindTextureCount++;
				}
				else
				{
					ID3D11ShaderResourceView* pNull = NULL;
					pContext->PSSetShaderResources(iTextureIdx, 1, &pNull);
				}
			}
		}

		if (geomInstance.Geometry != iLastGeometryId)
		{
			FE_ASSERT(Geometries.GetSize() >= geomInstance.Geometry, "Invalid geometry Id !");
			pGeometryData = &Geometries[geomInstance.Geometry - 1];
			iLastGeometryId = geomInstance.Geometry;
				
			UINT offset = 0;
			Device.GetImmediateContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			Device.GetImmediateContext()->IASetVertexBuffers(0, 1, (ID3D11Buffer**)&pGeometryData->VertexBuffer, &pGeometryData->Stride, &offset);
			Device.GetImmediateContext()->IASetIndexBuffer((ID3D11Buffer*)pGeometryData->IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
			RenderDebugInfos.FrameBindGeometryCount++;
		}
			
		RenderDebugInfos.FrameDrawCallsCount++;
		Device.GetImmediateContext()->DrawIndexed(pGeometryData->IndexCount, 0, 0);

		// todo : Set properties to constant buffers
	}
}
void FeModuleRendering::RenderDebugText(const FeDt& fDt)
{
	auto pResourcesHandler = FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();

	static uint32 iFrameCount = 0;
	if (fDt.TotalMilliseconds == 0)
		return;

	if (++iFrameCount == UPDATE_RENDER_INFOS_FREQUENCY)
	{
		iFrameCount = 0;
		 
		RenderDebugInfos.Framerate = (1000 / fDt.TotalMilliseconds);
		RenderDebugInfos.CpuFrame = fDt.TotalMilliseconds;
		RenderDebugInfos.GpuFrame = 0;
		RenderDebugInfos.CpuWait = fDt.TotalCpuWaited;
		RenderDebugInfos.DrawCalls = RenderDebugInfos.FrameDrawCallsCount;
		RenderDebugInfos.EffectBind = RenderDebugInfos.FrameBindEffectCount;
		RenderDebugInfos.GeometryBind = RenderDebugInfos.FrameBindGeometryCount;
	}

	switch (CurrentDebugTextMode)
	{
		case FeEDebugRenderTextMode::Memory:
			{
			FeMemoryManager::StaticInstance.GetDebugInfos(DebugString, DEBUG_STRING_SIZE);

			size_t iTxtLength = strlen(DebugString);
			FeModuleRenderResourcesHandlerDebugInfos resourcesDebugInfos;
			pResourcesHandler->ComputeDebugInfos(resourcesDebugInfos);
			sprintf_s(&DebugString[iTxtLength], DEBUG_STRING_SIZE - iTxtLength, "\
Texture Count\t%d \n\
Texture Mem.\t%4.1f / %4.0f (MB) \n\
				",
				resourcesDebugInfos.LoadedTexturesCount, 
				(resourcesDebugInfos.LoadedTexturesCountSizeInMemory) / (1024.0f*1024.0f),
				(resourcesDebugInfos.TexturesPoolSize) / (1024.0f*1024.0f));
			}
			break;
		case FeEDebugRenderTextMode::Rendering:
		{
			uint32 fGpuFrameTime = 0;
			uint32 fFramerate = (1000 / fDt.TotalMilliseconds);

			sprintf_s(DebugString,
				"\
Mode          \t: %s\n\
Fps           \t: %d\n\
Cpu (ms)      \t: %d\n\
Gpu (ms)      \t: %d\n\
CpuWait (ms)\t: %d\n\
Draw calls\t: %d \n\
Effect bind\t: %d \n\
Geom. bind\t: %d \n\
Texture bind\t: %d \n\
				", 
				CONFIGSTR
				,RenderDebugInfos.Framerate		
				,RenderDebugInfos.CpuFrame		
				,RenderDebugInfos.GpuFrame		
				,RenderDebugInfos.CpuWait		
				,RenderDebugInfos.DrawCalls		
				,RenderDebugInfos.EffectBind		
				,RenderDebugInfos.GeometryBind
				, RenderDebugInfos.FrameBindTextureCount);
		}break;
	}

	wchar_t wc[DEBUG_STRING_SIZE*2 +1];
	size_t iWSize;
	mbstowcs_s(&iWSize, wc, DebugString, DEBUG_STRING_SIZE);

	static float fBorderSize = 0.5f;
	static float fOffset = 2.f;

	FontWrapper->DrawString(
		Device.GetImmediateContext(),
		wc,// String
		20.0f + fBorderSize,// Font size
		10.0f - fBorderSize*fOffset,// X position
		10.0f - fBorderSize*fOffset*2.0f,// Y position
		0xff000000,// Text color, 0xAaBbGgRr
		FW1_RESTORESTATE // Flags (for example FW1_RESTORESTATE to keep context states unchanged)
		);
	FontWrapper->DrawString(
		Device.GetImmediateContext(),
		wc,// String
		20.0f,// Font size
		10.0f,// X position
		10.0f,// Y position
		0xff00ffff,// Text color, 0xAaBbGgRr
		FW1_RESTORESTATE // Flags (for example FW1_RESTORESTATE to keep context states unchanged)
		);
}

void FeModuleRendering::SwitchDebugRenderTextMode()
{
	CurrentDebugTextMode = (FeEDebugRenderTextMode::Type)((CurrentDebugTextMode + 1) % FeEDebugRenderTextMode::Count);
}
FeRenderBatch& FeModuleRendering::CreateRenderBatch()
{
	FeRenderBatch& renderBatch = RegisteredRenderBatches.Add();
	renderBatch.Viewport = &DefaultViewport;

	return renderBatch;
}
