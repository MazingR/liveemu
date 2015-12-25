#include <pch.hpp>

#include <modulerenderer.hpp>
#include <common/memorymanager.hpp>

#include <windows.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include "FW1FontWrapper.h"
#include <SDL_mutex.h>
#include <SDL.h>

#define D3DFAILEDRETURN(func) { HRESULT ___hr = (func); if (___hr!=S_OK) return ___hr; }

namespace FeRendering
{
	struct CBNeverChanges
	{
		XMMATRIX mView;
	};

	struct CBChangeOnResize
	{
		XMMATRIX mProjection;
	};

	struct CBChangesEveryFrame
	{
		XMMATRIX mWorld;
		XMFLOAT4 vMeshColor;
	};

	uint32 FeModuleRenderResourcesHandler::Load(const ::FeCommon::FeModuleInit*)
	{
		return EFeReturnCode::Success;
	}
	uint32 FeModuleRenderResourcesHandler::Unload()
	{
		return EFeReturnCode::Success;
	}
	uint32 FeModuleRenderResourcesHandler::Update(const FeDt& fDt)
	{
		return EFeReturnCode::Success;
	}
	const FeRenderTexture* FeModuleRenderResourcesHandler::GetTexture(const FeRenderTextureId& textureId) const
	{
		TexturesMap::const_iterator it = Textures.find(textureId);
		return it != Textures.end() ? &it->second : NULL;

	}
	bool FeModuleRenderResourcesHandler::IsLoaded(const FeRenderTextureId& textureId)
	{
		const FeRenderTexture* pTexture = GetTexture(textureId);
		return pTexture ? pTexture->LoadingState == FeETextureLoadingState::Loaded : false ;
	}
	bool FeModuleRenderResourcesHandler::IsLoading(const FeRenderTextureId& textureId)
	{
		const FeRenderTexture* pTexture = GetTexture(textureId);
		return pTexture ? pTexture->LoadingState == FeETextureLoadingState::Loading : false;
	}
	uint32 FeModuleRenderResourcesHandler::LoadTexture(const char* szTexturePath, FeRenderTextureId* pTextureId)
	{
		char szPath[COMMON_PATH_SIZE];
		sprintf_s(szPath, szTexturePath);
		FeCommon::ToLower(szPath);

		*pTextureId = FeCommon::GenerateUIntIdFromString(szPath);
		
		if (!GetTexture(*pTextureId))
		{
			// todo: make all of it asynchone & thread safe
			Textures[*pTextureId] = FeRenderTexture(); // add texture to map
			FeRenderTexture& texture = Textures[*pTextureId];
			ZeroMemory(&texture, sizeof(FeRenderTexture));
			sprintf_s(texture.Path.Str, szPath);

			ID3D11Device* pD3DDevice = FeModuleRendering::GetDevice().GetD3DDevice();
			HRESULT hr;
			D3DX11CreateTextureFromFile(pD3DDevice, szPath, NULL, NULL, &texture.Resource, &hr);

			if (!FAILED(hr))
			{
				hr = pD3DDevice->CreateShaderResourceView(texture.Resource, NULL, &texture.SRV);
			}
			texture.LoadingState = (FAILED(hr)) ? FeETextureLoadingState::LoadFailed : FeETextureLoadingState::Loaded;

			
		}
		return EFeReturnCode::Success;
	}
	uint32 FeModuleRenderResourcesHandler::UnloadTexture(const FeRenderTextureId&)
	{
		// todo
		return EFeReturnCode::Success;
	}

	uint32 FeModuleRendering::Load(const ::FeCommon::FeModuleInit* initBase)
	{
		auto init = (FeModuleRenderingInit*)initBase;

		FE_FAILEDRETURN(Device.Initialize(init->WindowHandle));

		FeRenderEffect& newEffect = Effects.Add();
		FE_FAILEDRETURN(newEffect.CreateFromFile("../data/themes/common/shaders/default.fx"));

		// Creat geometries
		Geometries.Reserve(16);
		Geometries.SetZeroMemory();
		FeRenderGeometryData& geometryData = Geometries.Add();

		FeRenderGeometryId geometryId;
		FeGeometryHelper::CreateStaticGeometry(FeEGemetryDataType::Quad, &geometryData, &geometryId);

		// Creat textures
		auto pResourcesHandler = FeCommon::FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();
		FeRenderTextureId textureId;
		pResourcesHandler->LoadTexture("../data/image.jpg", &textureId);
		renderBatch.Viewport.CreateFromBackBuffer();

		for (uint32 i = 0; i < 2048; ++i)
		{
			// DEBUG code 
			
			FeRenderGeometryInstance& geomInstance = renderBatch.GeometryInstances.Add();

			geomInstance.Effect = 1;
			geomInstance.Geometry = geometryId;
			geomInstance.Textures.Add(textureId);
		}
		
		HRESULT hResult = FW1CreateFactory(FW1_VERSION, &FW1Factory);
		hResult = FW1Factory->CreateFontWrapper(Device.GetD3DDevice(), L"Arial", &FontWrapper);

		CurrentDebugTextMode = FeEDebugRenderTextMode::Memory;

		//D3D11_QUERY_DESC desc;
		//ZeroMemory(&desc, sizeof(D3D11_QUERY_DESC));
		//ID3D11Query* pQuery;
		//desc.Query = D3D11_QUERY_TIMESTAMP;
		//Device.GetD3DDevice()->CreateQuery(&desc, &pQuery);

		ZeroMemory(&RenderDebugInfos, sizeof(FeRenderDebugInfos));

		return EFeReturnCode::Success;
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

		return EFeReturnCode::Success;
	}
	uint32 FeModuleRendering::Update(const FeDt& fDt)
	{
		int iProcessedMsg = 0;
		int iMaxProcessedMsg = 3;

		MSG msg = { 0 };

		while (WM_QUIT != msg.message && iProcessedMsg++<iMaxProcessedMsg)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				BeginRender();
				RenderBatch(renderBatch, fDt);
				RenderDebugText(fDt);
				EndRender();
			}
		}

		if (WM_QUIT == msg.message)
		{
			return EFeReturnCode::Canceled;
		}

		return EFeReturnCode::Success;
	}
	void FeModuleRendering::BeginRender()
	{
		RenderDebugInfos.FrameDrawCallsCount = 0;
		RenderDebugInfos.FrameBindEffectCount = 0;
		RenderDebugInfos.FrameBindGeometryCount = 0;
		
	}
	void FeModuleRendering::EndRender()
	{
		uint32 iTicks = SDL_GetTicks();
		// Present the information rendered to the back buffer to the front buffer (the screen)
		Device.GetSwapChain()->Present(0, 0);
	}
	void FeModuleRendering::RenderBatch(FeRenderBatch& batch, const FeDt& fDt)
	{
		// Clear the back buffer
		batch.Viewport.Clear();
		batch.Viewport.Bind();

		FeCommon::FeTArray<FeRenderGeometryInstance>& instances = batch.GeometryInstances;

		FeRenderEffectId iLastEffectId = 0;
		FeRenderGeometryId iLastGeometryId = 0;
		
		FeRenderGeometryData* pGeometryData = NULL;
		FeRenderEffect* pEffect = NULL;
		FeRenderCamera camera;

		auto pResourcesHandler = FeCommon::FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();
		ID3D11DeviceContext* pContext = GetDevice().GetImmediateContext();

		static float fRotX = 0, fRotY = 0, fRotZ = 0;
		static FeVector3 translation(0,0,0), scale(1,1,1);
		static float fOffset = 0.0f;
		float fOffsetBetween = 1.5f;
		uint32 iColomns = 20;
		fOffset += 0.001f;

		translation.mData[0] = -iColomns;
		translation.mData[1] = 0;
		translation.mData[2] = 50.0f-fOffset;

		scale.mData[0] = 1.0f + fOffset;
		scale.mData[1] = 1.0f + fOffset;

		fRotZ = fOffset*5.0f;
		FeRenderTextureId lastBindedTextures[16];
		ZeroMemory(lastBindedTextures, sizeof(FeRenderTextureId) * 16);

		for (uint32 i = 0; i < Effects.GetSize() ; ++i)
		{
			Effects[i].BeginFrame(camera, batch.Viewport);
		}

		for (uint32 iInstanceIdx = 0; iInstanceIdx < instances.GetSize(); ++iInstanceIdx)
		{
			FeRenderGeometryInstance& geomInstance = instances[iInstanceIdx];
			translation.mData[0] = -10 + (iInstanceIdx % iColomns) * fOffsetBetween;
			translation.mData[1] = -20 + (iInstanceIdx / iColomns) * fOffsetBetween;

			FeGeometryHelper::ComputeAffineTransform(geomInstance.Transform, translation, FeRotation(fRotX, fRotY, fRotZ), scale);

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
					if (pTexture)
						pContext->PSSetShaderResources(iTextureIdx, 1, &pTexture->SRV);
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

			// todo : Set properties
		}
	}
	void FeModuleRendering::RenderDebugText(const FeDt& fDt)
	{
		static uint32 iFrameCount = 0;
		if (fDt.TotalMilliseconds == 0)
			return;

		RenderDebugInfos.Framerate		+= (1000 / fDt.TotalMilliseconds);
		RenderDebugInfos.CpuFrame		+= fDt.TotalMilliseconds;
		RenderDebugInfos.GpuFrame		+= 0;
		RenderDebugInfos.CpuWait		+= fDt.TotalCpuWaited;
		RenderDebugInfos.DrawCalls		= RenderDebugInfos.FrameDrawCallsCount;
		RenderDebugInfos.EffectBind		= RenderDebugInfos.FrameBindEffectCount;
		RenderDebugInfos.GeometryBind	= RenderDebugInfos.FrameBindGeometryCount;

		if (iFrameCount > 0)
		{
			RenderDebugInfos.Framerate		/=2;
			RenderDebugInfos.CpuFrame		/=2;
			RenderDebugInfos.GpuFrame		/=2;
			RenderDebugInfos.CpuWait		/=2;
		}
		iFrameCount++;
		if (iFrameCount > 256)
			iFrameCount = 0;

		switch (CurrentDebugTextMode)
		{
			case FeEDebugRenderTextMode::Memory:
				FeCommon::FeMemoryManager::StaticInstance.GetDebugInfos(DebugString, DEBUG_STRING_SIZE);
				break;
			case FeEDebugRenderTextMode::Rendering:
			{
				uint32 fGpuFrameTime = 0;
				uint32 fFramerate = (1000 / fDt.TotalMilliseconds);

				sprintf_s(DebugString,
					"\
Mode\t: %s\n\
Framerate\t: %d (fps)\n\
Cpu Frame\t: %d (ms)\n\
Gpu Frame\t: %d (ms)\n\
Cpu wait \t: %d (ms)\n\
Draw calls\t: %d \n\
Effect bind\t: %d \n\
Geometry bind\t: %d \n\
					", 
					CONFIGSTR
					,RenderDebugInfos.Framerate		
					,RenderDebugInfos.CpuFrame		
					,RenderDebugInfos.GpuFrame		
					,RenderDebugInfos.CpuWait		
					,RenderDebugInfos.DrawCalls		
					,RenderDebugInfos.EffectBind		
					,RenderDebugInfos.GeometryBind	);
			}break;
		}

		wchar_t wc[DEBUG_STRING_SIZE*2 +1];
		size_t iWSize;
		mbstowcs_s(&iWSize, wc, DebugString, DEBUG_STRING_SIZE);

		FontWrapper->DrawString(
			Device.GetImmediateContext(),
			wc,// String
			13.0f,// Font size
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
} // namespace FeRendering
