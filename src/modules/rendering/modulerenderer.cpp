#include <pch.hpp>

#include <modulerenderer.hpp>
#include <common/memorymanager.hpp>

#include <windows.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>
#include "FW1FontWrapper.h"
#include <SDL_mutex.h>

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
	uint32 FeModuleRenderResourcesHandler::Update(float fDt)
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

		// DEBUG code 
		FeRenderGeometryId geometryId;

		auto pResourcesHandler = FeCommon::FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();
		FeRenderTextureId textureId;
		pResourcesHandler->LoadTexture("../data/image.jpg", &textureId);

		Geometries.Reserve(16);
		Geometries.SetZeroMemory();

		FeRenderGeometryData& geometryData = Geometries.Add();

		FeGeometryHelper::CreateStaticGeometry(FeEGemetryDataType::Quad, &geometryData, &geometryId);

		renderBatch.Viewport.CreateFromBackBuffer();
		FeRenderGeometryInstance& geomInstance = renderBatch.GeometryInstances.Add();

		geomInstance.Effect = 1;
		geomInstance.Geometry = geometryId;
		geomInstance.Textures.Add(textureId);
		
		HRESULT hResult = FW1CreateFactory(FW1_VERSION, &FW1Factory);
		hResult = FW1Factory->CreateFontWrapper(Device.GetD3DDevice(), L"Arial", &FontWrapper);

		CurrentDebugTextMode = FeEDebugRenderTextMode::Memory;

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
	uint32 FeModuleRendering::Update(float fDt)
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
				RenderDebugText();
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

	}
	void FeModuleRendering::EndRender()
	{
		// Present the information rendered to the back buffer to the front buffer (the screen)
		Device.GetSwapChain()->Present(0, 0);
	}
	void FeModuleRendering::RenderBatch(FeRenderBatch& batch, float fDt)
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
		static float fRotX = 0, fRotY = 0, fRotZ = 0;
		static FeVector3 translation(0,0,10), scale(1,1,1);
		static float fDelta = 0.5f;

		translation.mData[2] -= fDelta*fDt;
		scale.mData[0] += fDelta*fDt;
		scale.mData[1] += fDelta*fDt;
		fRotZ += fDelta*fDt*2.0f;

		for (uint32 i = 0; i < Effects.GetSize(); ++i)
		{
			Effects[i].BeginFrame(camera, batch.Viewport);
		}
		for (uint32 iInstanceIdx = 0; iInstanceIdx < instances.GetSize(); ++iInstanceIdx)
		{
			FeRenderGeometryInstance& geomInstance = instances[iInstanceIdx];
			
			FeGeometryHelper::ComputeAffineTransform(geomInstance.Transform, translation, FeRotation(fRotX, fRotY, fRotZ), scale);

			if (geomInstance.Effect != iLastEffectId)
			{
				FE_ASSERT(Effects.GetSize() >= geomInstance.Effect, "Invalid effect Id !");
				pEffect = &Effects[geomInstance.Effect - 1];
				pEffect->Bind();
				iLastEffectId = geomInstance.Effect;
			}

			pEffect->BindGeometryInstance(geomInstance, pResourcesHandler);

			if (geomInstance.Geometry != iLastGeometryId)
			{
				FE_ASSERT(Geometries.GetSize() >= geomInstance.Geometry, "Invalid geometry Id !");
				pGeometryData = &Geometries[geomInstance.Geometry - 1];
				iLastGeometryId = geomInstance.Geometry;
				
				UINT offset = 0;
				Device.GetImmediateContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				Device.GetImmediateContext()->IASetVertexBuffers(0, 1, (ID3D11Buffer**)&pGeometryData->VertexBuffer, &pGeometryData->Stride, &offset);
				Device.GetImmediateContext()->IASetIndexBuffer((ID3D11Buffer*)pGeometryData->IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
			}

			Device.GetImmediateContext()->DrawIndexed(pGeometryData->IndexCount, 0, 0);

			// todo : Set properties
		}
	}
	void FeModuleRendering::RenderDebugText()
	{
		switch (CurrentDebugTextMode)
		{
			case FeEDebugRenderTextMode::Memory:
				FeCommon::FeMemoryManager::StaticInstance.GetDebugInfos(DebugString, DEBUG_STRING_SIZE);
				break;
			case FeEDebugRenderTextMode::Rendering:
			{
				float fGpuFrameTime = 0.0f;
				float fCpuFrameTime = 0.0f;
				uint32 fFramerate = 60;

				sprintf_s(DebugString,
					"\
					Mode\t: %s\n\
					Framerate\t: %d (fps)\n\
					Cpu Frame\t: %4.2f (ms)\n\
					Gpu Frame\t: %4.2f (ms)\n\
					", CONFIGSTR, fFramerate, fCpuFrameTime, fGpuFrameTime);
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

} // namespace FeRendering
