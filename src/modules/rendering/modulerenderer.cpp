#include <modulerenderer.hpp>
#include <renderresourceshandler.hpp>
#include <common/memorymanager.hpp>
#include <common/filesystem.hpp>

#include <d3dx11include.hpp>
#include "FW1FontWrapper.h"
#include <SDL.h>

#define UPDATE_RENDER_INFOS_FREQUENCY 30
#define D3DFAILEDRETURN(func) { HRESULT ___hr = (func); if (___hr!=S_OK) return ___hr; }

void OnEffectFileChanged(FeEFileChangeType::Type eChangeType, const char* szPath, void* pUserData)
{
	((FeModuleRendering*)pUserData)->ReloadEffects();
}
uint32 FeModuleRendering::ReloadEffects()
{
	for (auto& effect : Effects)
	{
		effect.second.Release();
		FE_FAILEDRETURN(effect.second.CreateFromFile(effect.second.GetFile().Value));
	}

	return FeEReturnCode::Success;
}
void FeModuleRendering::UnloadEffects()
{
	for (auto& effect : Effects)
	{
		effect.second.Release();
	}
	Effects.clear();
}
uint32 FeModuleRendering::LoadEffects(const FeTArray<FeRenderEffect>& effects)
{
	for (auto& newEffect : effects)
	{
		uint32 iId = newEffect.GetName().Id();

		FE_ASSERT(Effects.find(iId) == Effects.end(), "Trying to add effect with same name : %s", newEffect.GetName().Cstr());
		Effects[iId] = newEffect;

		FeRenderEffect& effect = Effects[iId];
		if (effect.CreateFromFile(effect.GetFile().Value) != FeEReturnCode::Success)
		{
			Effects.erase(iId);
		}
	}

	return FeEReturnCode::Success;
}
uint32 FeModuleRendering::Load(const FeModuleInit* initBase)
{
	RegisteredRenderBatches.SetHeapId(RENDERER_HEAP);

	auto init = (FeModuleRenderingInit*)initBase;

	RenderInit = *init;

	FE_FAILEDRETURN(Device.Initialize(init->WindowHandle));

	// Creat static geometries (primitive forms)
	Geometries.Reserve(16);
	Geometries.SetZeroMemory();
	FeRenderGeometryData& geometryData = Geometries.Add();
	FeResourceId geometryId;
	FeGeometryHelper::CreateStaticGeometry(FeEGemetryDataType::Quad, &geometryData, &geometryId);

	HRESULT hResult = FW1CreateFactory(FW1_VERSION, &FW1Factory);
	hResult = FW1Factory->CreateFontWrapper(Device.GetD3DDevice(), L"Areial", &FontWrapper);

	CurrentDebugTextMode = FeEDebugRenderTextMode::Rendering;

	ZeroMemory(&RenderDebugInfos, sizeof(FeRenderDebugInfos));

	DefaultViewport.CreateFromBackBuffer();

	auto pFileManagerModule = FeApplication::StaticInstance.GetModule<FeModuleFilesManager>();
	pFileManagerModule->WatchDirectory("../data/themes/common/shaders", OnEffectFileChanged, this);

	return FeEReturnCode::Success;
}
uint32 FeModuleRendering::Unload()
{
	UnloadEffects();

	FeGeometryHelper::ReleaseGeometryData();
	FeGeometryHelper::ReleaseStaticGeometryData();

	SafeRelease(FontWrapper);
	SafeRelease(FW1Factory);

	Device.Release();

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
		for (  auto& batch : RegisteredRenderBatches)
		{
			RenderBatch(batch, fDt);
			batch.GeometryInstances.Clear();
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

	FeResourceId iLastEffectId = 0;
	FeResourceId iLastGeometryId = 0;
		
	FeRenderGeometryData* pGeometryData = NULL;
	FeRenderEffect* pEffect = NULL;
	FeRenderCamera camera;

	auto pResourcesHandler = FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();
	ID3D11DeviceContext* pContext = GetDevice().GetImmediateContext();

	FeResourceId lastBindedTextures[16];
	ZeroMemory(lastBindedTextures, sizeof(FeResourceId) * 16);

	for (auto& effect : Effects)
	{
		effect.second.BeginFrame(camera, *batch.Viewport, fDt.TotalSeconds);
	}

	for (uint32 iInstanceIdx = 0; iInstanceIdx < instances.GetSize(); ++iInstanceIdx)
	{
		FeRenderGeometryInstance& geomInstance = instances[iInstanceIdx];
		
		if (geomInstance.Effect != iLastEffectId && Effects.find(geomInstance.Effect) != Effects.end())
		{
			pEffect = &Effects[geomInstance.Effect];
			pEffect->Bind();
			iLastEffectId = geomInstance.Effect;

			RenderDebugInfos.FrameBindEffectCount++;
		}
		if (!pEffect)
			continue; // effect not found !

		pEffect->BindGeometryInstance(geomInstance, pResourcesHandler);

		// Set resources (textures)
		for (uint32 iTextureIdx = 0; iTextureIdx < pEffect->GetTextureLevels() ; ++iTextureIdx)
		{
			bool bBinded = false;

			if (geomInstance.Textures.GetSize() > iTextureIdx)
			{
				const FeResourceId& textureId = geomInstance.Textures[iTextureIdx];

				if (lastBindedTextures[iTextureIdx] != textureId)
				{
					lastBindedTextures[iTextureIdx] = textureId;
					const FeRenderResource* pResource = pResourcesHandler->GetResource(textureId);

					if (pResource && pResource->LoadingState == FeEResourceLoadingState::Loaded)
					{
						if (pResource->Type == FeEResourceType::Texture)
						{
							FeRenderTexture* pTexData = (FeRenderTexture*)pResource->Interface->GetData();
							pContext->PSSetShaderResources(iTextureIdx, 1, &pTexData->D3DSRV);
							bBinded = true;
						}
						else if (pResource->Type == FeEResourceType::Font)
						{
							FeRenderFont* pFontData = (FeRenderFont*)pResource->Interface->GetData();
							pContext->PSSetShaderResources(iTextureIdx, 1, &pFontData->Texture.D3DSRV);
							bBinded = true;
						}
						RenderDebugInfos.FrameBindTextureCount++;
					}
				}
			}
			if (!bBinded)
			{
				static ID3D11ShaderResourceView* pNull = NULL;
				pContext->PSSetShaderResources(iTextureIdx, 1, &pNull);
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

void DrawTextLines(const char* szText, uint32& iLinesCount, uint32 iLineLength, float fFontSize, uint32 iColor, IFW1FontWrapper* pFontWrapper, ID3D11DeviceContext *pContext)
{
	wchar_t szWTmp[DEBUG_STRING_SIZE];
	char szTmp[DEBUG_STRING_SIZE];
	
	uint32 iNewLinesCount = 0;
	uint32 iTextLen = strlen(szText);
	uint32 iLineStart = 0;
	uint32 iLineEnd = FeStringTools::IndexOf(szText, '\n');
	uint32 iLineSize = iLineEnd - iLineStart;

	memset(szTmp, 0, DEBUG_STRING_SIZE);
	memcpy_s(szTmp, DEBUG_STRING_SIZE, szText, iTextLen);

	while (iLineStart < iTextLen && iLineLength<iTextLen)
	{
		bool bCut = false;

		// didn't find a '\n'
		if ((size_t)-1 == iLineEnd)
		{
			iLineStart += min(iTextLen - iLineStart, iLineLength);
			bCut = true;
		}
		else if (iLineSize <= iLineLength)
		{
			iLineStart = iLineEnd + 1;
		}
		// '\n' is too far, cut the line
		else
		{
			iLineStart += iLineLength;
			bCut = true;
		}
		if (bCut)
		{
			szTmp[iLineStart] = '\n';
			memcpy_s(szTmp + iLineStart + 1, DEBUG_STRING_SIZE - iLineStart - 1, szText + iLineStart, iTextLen - iLineStart);
		}
		iNewLinesCount++;

		if (iLineStart + iLineLength >= iTextLen)
		{
			iNewLinesCount++;
			break;
		}

		iLineEnd = FeStringTools::IndexOf(szText, '\n', iLineStart);
		iLineSize = iLineEnd - iLineStart;
	}
	size_t iWSize;
	mbstowcs_s(&iWSize, szWTmp, szTmp, DEBUG_STRING_SIZE);
	pFontWrapper->DrawString(
		pContext,
		szWTmp,// String
		fFontSize,// Font size
		10.0f,// X position
		iLinesCount*fFontSize,// Y position
		iColor,// Text color, 0xAaBbGgRr
		FW1_RESTORESTATE // Flags (for example FW1_RESTORESTATE to keep context states unchanged)
		);

	iLinesCount += iNewLinesCount;
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
				resourcesDebugInfos.LoadedResourcesCount, 
				(resourcesDebugInfos.LoadedResourcesCountSizeInMemory) / (1024.0f*1024.0f),
				(resourcesDebugInfos.ResourcesPoolSize) / (1024.0f*1024.0f));
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

	uint32 iLinesCount = 0;
	uint32 iLineLength = (RenderInit.Width/15)*2 -1;
	DrawTextLines(FeGetLastError(), iLinesCount, iLineLength, 15.0f, 0xff0000ff, FontWrapper, Device.GetImmediateContext());
	iLinesCount += 3;
	DrawTextLines(DebugString, iLinesCount, iLineLength, 15.0f, 0xff00ffff, FontWrapper, Device.GetImmediateContext());
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
