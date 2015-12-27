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
	uint32 FeModuleRenderResourcesHandler::ComputeTextureSizeInMemoryFromFormat(uint32 iWidth, uint32 iHeight, uint32 iTextureFormat, bool bHasAlpha)
	{
		uint32 iTextureSize = 0;
		DXGI_FORMAT iFormat = (DXGI_FORMAT)iTextureFormat;
		uint32 iPixelBitSize = 0;
		
		switch(iFormat)
		{
			case DXGI_FORMAT_BC1_TYPELESS				:	iPixelBitSize = 8;		break;
			case DXGI_FORMAT_BC1_UNORM					:	iPixelBitSize = 8;		break;
			case DXGI_FORMAT_BC1_UNORM_SRGB				:	iPixelBitSize = 8;		break;

			case DXGI_FORMAT_BC2_TYPELESS				:	iPixelBitSize = 16;	break;
			case DXGI_FORMAT_BC2_UNORM					:	iPixelBitSize = 16;	break;
			case DXGI_FORMAT_BC2_UNORM_SRGB				:	iPixelBitSize = 16;	break;

			case DXGI_FORMAT_BC3_TYPELESS				:	iPixelBitSize = 16;	break;
			case DXGI_FORMAT_BC3_UNORM					:	iPixelBitSize = 16;	break;
			case DXGI_FORMAT_BC3_UNORM_SRGB				:	iPixelBitSize = 16;	break;

			case DXGI_FORMAT_BC4_TYPELESS				:	iPixelBitSize = 8;		break;
			case DXGI_FORMAT_BC4_UNORM					:	iPixelBitSize = 8;		break;
			case DXGI_FORMAT_BC4_SNORM					:	iPixelBitSize = 8;		break;

			case DXGI_FORMAT_BC5_TYPELESS				:	iPixelBitSize = 16;	break;
			case DXGI_FORMAT_BC5_UNORM					:	iPixelBitSize = 16;	break;
			case DXGI_FORMAT_BC5_SNORM					:	iPixelBitSize = 16;	break;

			case DXGI_FORMAT_B5G6R5_UNORM				:	iPixelBitSize = 16;	break;
			case DXGI_FORMAT_B5G5R5A1_UNORM				:	iPixelBitSize = 16;	break;

			case DXGI_FORMAT_BC6H_TYPELESS				:	iPixelBitSize = 0;		break;
			case DXGI_FORMAT_BC6H_UF16					:	iPixelBitSize = 0;		break;
			case DXGI_FORMAT_BC6H_SF16					:	iPixelBitSize = 0;		break;
			case DXGI_FORMAT_BC7_TYPELESS				:	iPixelBitSize = 0;		break;
			case DXGI_FORMAT_BC7_UNORM					:	iPixelBitSize = 0;		break;
			case DXGI_FORMAT_BC7_UNORM_SRGB				:	iPixelBitSize = 0;		break;

			default:
				iPixelBitSize = 0;
		};

		if (iPixelBitSize != 0) // texture format is compressed
		{
			iTextureSize = ((iWidth*iHeight) / 16)*(iPixelBitSize / 4);
		}
		else
		{
			switch(iFormat)
			{
				case DXGI_FORMAT_UNKNOWN					:	iPixelBitSize = 0;		break;

				case DXGI_FORMAT_R32G32B32A32_TYPELESS		:	iPixelBitSize = 32*4;	break;
				case DXGI_FORMAT_R32G32B32A32_FLOAT			:	iPixelBitSize = 32*4;	break;
				case DXGI_FORMAT_R32G32B32A32_UINT			:	iPixelBitSize = 32*4;	break;
				case DXGI_FORMAT_R32G32B32A32_SINT			:	iPixelBitSize = 32*4;	break;

				case DXGI_FORMAT_R32G32B32_TYPELESS			:	iPixelBitSize = 32*3;	break;
				case DXGI_FORMAT_R32G32B32_FLOAT			:	iPixelBitSize = 32*3;	break;
				case DXGI_FORMAT_R32G32B32_UINT				:	iPixelBitSize = 32*3;	break;
				case DXGI_FORMAT_R32G32B32_SINT				:	iPixelBitSize = 32*3;	break;

				case DXGI_FORMAT_R16G16B16A16_TYPELESS		:	iPixelBitSize = 16*4;	break;
				case DXGI_FORMAT_R16G16B16A16_FLOAT			:	iPixelBitSize = 16*4;	break;
				case DXGI_FORMAT_R16G16B16A16_UNORM			:	iPixelBitSize = 16*4;	break;
				case DXGI_FORMAT_R16G16B16A16_UINT			:	iPixelBitSize = 16*4;	break;
				case DXGI_FORMAT_R16G16B16A16_SNORM			:	iPixelBitSize = 16*4;	break;
				case DXGI_FORMAT_R16G16B16A16_SINT			:	iPixelBitSize = 16*4;	break;

				case DXGI_FORMAT_R32G32_TYPELESS			:	iPixelBitSize = 32*2;	break;
				case DXGI_FORMAT_R32G32_FLOAT				:	iPixelBitSize = 32*2;	break;
				case DXGI_FORMAT_R32G32_UINT				:	iPixelBitSize = 32*2;	break;
				case DXGI_FORMAT_R32G32_SINT				:	iPixelBitSize = 32*2;	break;

				case DXGI_FORMAT_R32G8X24_TYPELESS			:	iPixelBitSize = 8;		break;
				case DXGI_FORMAT_D32_FLOAT_S8X24_UINT		:	iPixelBitSize = 8;		break;
				case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS	:	iPixelBitSize = 8;		break;
				case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT	:	iPixelBitSize = 8;		break;
				case DXGI_FORMAT_R10G10B10A2_TYPELESS		:	iPixelBitSize = 8;		break;
				case DXGI_FORMAT_R10G10B10A2_UNORM			:	iPixelBitSize = 8;		break;
				case DXGI_FORMAT_R10G10B10A2_UINT			:	iPixelBitSize = 8;		break;
				case DXGI_FORMAT_R11G11B10_FLOAT			:	iPixelBitSize = 8;		break;

				case DXGI_FORMAT_R8G8B8A8_TYPELESS			:	iPixelBitSize = 8*(bHasAlpha?4:3);	break;
				case DXGI_FORMAT_R8G8B8A8_UNORM				:	iPixelBitSize = 8*(bHasAlpha?4:3);	break;
				case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB		:	iPixelBitSize = 8*(bHasAlpha?4:3);	break;
				case DXGI_FORMAT_R8G8B8A8_UINT				:	iPixelBitSize = 8*(bHasAlpha?4:3);	break;
				case DXGI_FORMAT_R8G8B8A8_SNORM				:	iPixelBitSize = 8*(bHasAlpha?4:3);	break;
				case DXGI_FORMAT_R8G8B8A8_SINT				:	iPixelBitSize = 8*(bHasAlpha?4:3);	break;

				case DXGI_FORMAT_R16G16_TYPELESS			:	iPixelBitSize = 16*2;	break;
				case DXGI_FORMAT_R16G16_FLOAT				:	iPixelBitSize = 16*2;	break;
				case DXGI_FORMAT_R16G16_UNORM				:	iPixelBitSize = 16*2;	break;
				case DXGI_FORMAT_R16G16_UINT				:	iPixelBitSize = 16*2;	break;
				case DXGI_FORMAT_R16G16_SNORM				:	iPixelBitSize = 16*2;	break;
				case DXGI_FORMAT_R16G16_SINT				:	iPixelBitSize = 16*2;	break;

				case DXGI_FORMAT_R32_TYPELESS				:	iPixelBitSize = 32;	break;
				case DXGI_FORMAT_D32_FLOAT					:	iPixelBitSize = 32;	break;
				case DXGI_FORMAT_R32_FLOAT					:	iPixelBitSize = 32;	break;
				case DXGI_FORMAT_R32_UINT					:	iPixelBitSize = 32;	break;
				case DXGI_FORMAT_R32_SINT					:	iPixelBitSize = 32;	break;

				case DXGI_FORMAT_R24G8_TYPELESS				:	iPixelBitSize = 8;		break;
				case DXGI_FORMAT_D24_UNORM_S8_UINT			:	iPixelBitSize = 8;		break;
				case DXGI_FORMAT_R24_UNORM_X8_TYPELESS		:	iPixelBitSize = 8;		break;
				case DXGI_FORMAT_X24_TYPELESS_G8_UINT		:	iPixelBitSize = 8;		break;

				case DXGI_FORMAT_R8G8_TYPELESS				:	iPixelBitSize = 8*2;	break;
				case DXGI_FORMAT_R8G8_UNORM					:	iPixelBitSize = 8*2;	break;
				case DXGI_FORMAT_R8G8_UINT					:	iPixelBitSize = 8*2;	break;
				case DXGI_FORMAT_R8G8_SNORM					:	iPixelBitSize = 8*2;	break;
				case DXGI_FORMAT_R8G8_SINT					:	iPixelBitSize = 8*2;	break;

				case DXGI_FORMAT_R16_TYPELESS				:	iPixelBitSize = 16;	break;
				case DXGI_FORMAT_R16_FLOAT					:	iPixelBitSize = 16;	break;
				case DXGI_FORMAT_D16_UNORM					:	iPixelBitSize = 16;	break;
				case DXGI_FORMAT_R16_UNORM					:	iPixelBitSize = 16;	break;
				case DXGI_FORMAT_R16_UINT					:	iPixelBitSize = 16;	break;
				case DXGI_FORMAT_R16_SNORM					:	iPixelBitSize = 16;	break;
				case DXGI_FORMAT_R16_SINT					:	iPixelBitSize = 16;	break;

				case DXGI_FORMAT_R8_TYPELESS				:	iPixelBitSize = 8;		break;
				case DXGI_FORMAT_R8_UNORM					:	iPixelBitSize = 8;		break;
				case DXGI_FORMAT_R8_UINT					:	iPixelBitSize = 8;		break;
				case DXGI_FORMAT_R8_SNORM					:	iPixelBitSize = 8;		break;
				case DXGI_FORMAT_R8_SINT					:	iPixelBitSize = 8;		break;
				case DXGI_FORMAT_A8_UNORM					:	iPixelBitSize = 8;		break;

				case DXGI_FORMAT_R1_UNORM					:	iPixelBitSize = 8;		break;
				case DXGI_FORMAT_R9G9B9E5_SHAREDEXP			:	iPixelBitSize = 8;		break;
				case DXGI_FORMAT_R8G8_B8G8_UNORM			:	iPixelBitSize = 8;		break;
				case DXGI_FORMAT_G8R8_G8B8_UNORM			:	iPixelBitSize = 8;		break;

				case DXGI_FORMAT_B8G8R8A8_UNORM				:	iPixelBitSize = 8*(bHasAlpha?4:3);	break;
				case DXGI_FORMAT_B8G8R8X8_UNORM				:	iPixelBitSize = 8*4;	break;
				case DXGI_FORMAT_B8G8R8A8_TYPELESS			:	iPixelBitSize = 8*(bHasAlpha?4:3);	break;
				case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB		:	iPixelBitSize = 8*(bHasAlpha?4:3);	break;
				case DXGI_FORMAT_B8G8R8X8_TYPELESS			:	iPixelBitSize = 8*4;	break;
				case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB		:	iPixelBitSize = 8*4;	break;

				case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:	iPixelBitSize = 8;		break;
				case DXGI_FORMAT_FORCE_UINT					:	iPixelBitSize = 8;		break;

				default:
					iPixelBitSize = 0;
			};
		}
		iTextureSize = (iWidth*iHeight)*(iPixelBitSize / 4);

		return iTextureSize;
	}
	void FeModuleRenderResourcesHandler::ComputeDebugInfos(FeModuleRenderResourcesHandlerDebugInfos& infos)
	{
		infos.LoadedTexturesCount = 0;
		infos.LoadedTexturesCountSizeInMemory = 0;

		for (TexturesMapIt it = Textures.begin(); it != Textures.end(); ++it)
		{
			FeRenderTexture& texture = it->second;

			if (texture.LoadingState == FeETextureLoadingState::Loaded)
			{
				infos.LoadedTexturesCount++;
				infos.LoadedTexturesCountSizeInMemory += texture.SizeInMemory;
			}
		}
			
	}
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
			D3DX11_IMAGE_LOAD_INFO loadinfos;
			D3DX11_IMAGE_INFO imgInfos;
			ZeroMemory(&loadinfos, sizeof(D3DX11_IMAGE_LOAD_INFO));

			D3DX11GetImageInfoFromFile(szPath, NULL, &imgInfos, &hr);
			
			loadinfos.Width = imgInfos.Width;
			loadinfos.Height = imgInfos.Height;
			loadinfos.Depth = imgInfos.Depth;
			loadinfos.FirstMipLevel = 0;
			loadinfos.MipLevels = 1;
			loadinfos.Usage = D3D11_USAGE_DEFAULT;
			loadinfos.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			loadinfos.CpuAccessFlags = 0;
			loadinfos.MiscFlags = 0;
			loadinfos.Format = DXGI_FORMAT_BC1_UNORM;// imgInfos.Format;
			loadinfos.Filter = D3DX11_FILTER_LINEAR;
			loadinfos.MipFilter = D3DX11_FILTER_LINEAR;
			loadinfos.pSrcInfo = &imgInfos;

			

			D3DX11CreateTextureFromFile(pD3DDevice, szPath, &loadinfos, NULL, &texture.Resource, &hr);

			if (SUCCEEDED(hr))
			{
				hr = pD3DDevice->CreateShaderResourceView(texture.Resource, NULL, &texture.SRV);
				// compute size in memory
				ID3D11Texture2D *pTextureInterface = 0;
				ID3D11Resource *res;
				texture.Resource->QueryInterface<ID3D11Texture2D>(&pTextureInterface);
				D3D11_TEXTURE2D_DESC desc;
				pTextureInterface->GetDesc(&desc);

				texture.SizeInMemory = ComputeTextureSizeInMemoryFromFormat(desc.Width, desc.Height, desc.Format, true);

				texture.LoadingState = FeETextureLoadingState::Loaded;
			}
			else
			{
				FE_ASSERT(false, "texture  loading failed");
				texture.LoadingState = FeETextureLoadingState::LoadFailed;
			}
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

		// Process iMaxProcessedMsg at maximum
		while (WM_QUIT != msg.message && iProcessedMsg++<iMaxProcessedMsg)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

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
			return EFeReturnCode::Canceled;
		}

		return EFeReturnCode::Success;
	}
	void FeModuleRendering::BeginRender()
	{
		RenderDebugInfos.FrameDrawCallsCount = 0;
		RenderDebugInfos.FrameBindEffectCount = 0;
		RenderDebugInfos.FrameBindGeometryCount = 0;
		RenderDebugInfos.FrameBindTextureCount = 0;
		
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
		batch.Viewport->Clear();
		batch.Viewport->Bind();

		FeCommon::FeTArray<FeRenderGeometryInstance>& instances = batch.GeometryInstances;

		FeRenderEffectId iLastEffectId = 0;
		FeRenderGeometryId iLastGeometryId = 0;
		
		FeRenderGeometryData* pGeometryData = NULL;
		FeRenderEffect* pEffect = NULL;
		FeRenderCamera camera;

		auto pResourcesHandler = FeCommon::FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();
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
		auto pResourcesHandler = FeCommon::FeApplication::StaticInstance.GetModule<FeModuleRenderResourcesHandler>();

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
		if (iFrameCount > 32)
			iFrameCount = 0;

		switch (CurrentDebugTextMode)
		{
			case FeEDebugRenderTextMode::Memory:
				{
				FeCommon::FeMemoryManager::StaticInstance.GetDebugInfos(DebugString, DEBUG_STRING_SIZE);

				size_t iTxtLength = strlen(DebugString);
				FeModuleRenderResourcesHandlerDebugInfos resourcesDebugInfos;
				pResourcesHandler->ComputeDebugInfos(resourcesDebugInfos);
				sprintf_s(&DebugString[iTxtLength], DEBUG_STRING_SIZE - iTxtLength, "\
Texture Count\t%d \n\
Texture Mem.\t%4.2f (MB) \n\
					",
					resourcesDebugInfos.LoadedTexturesCount, 
					(resourcesDebugInfos.LoadedTexturesCountSizeInMemory) / (1024.0f*1024.0f));
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
} // namespace FeRendering
