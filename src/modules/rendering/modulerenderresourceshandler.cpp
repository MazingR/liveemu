#include <modulerenderresourceshandler.hpp>
#include <modulerenderer.hpp>

#include <common/memorymanager.hpp>
#include <common/filesystem.hpp>
#include <common/string.hpp>

#include <d3dx11include.hpp>
#include <SDL.h>

#define USE_DDS_IF_EXISTS 1
#define SAVE_CONVERTED_TO_DDS 1
#define D3DFAILEDRETURN(func) { HRESULT ___hr = (func); if (___hr!=S_OK) return ___hr; }

class FeScopeLockedMutex
{
public:
	SDL_mutex* Mutex;
	
	FeScopeLockedMutex(SDL_mutex* pMutex)
	{
		Mutex = pMutex;
		FE_ASSERT(Mutex, "FeScopeLockedMutex null mutex");
		SDL_LockMutex(Mutex);
	}
	~FeScopeLockedMutex()
	{
		SDL_UnlockMutex(Mutex);
	}
};
#define SCOPELOCK(mutex, name) FeScopeLockedMutex name(mutex);
#define SCOPELOCK(mutex) FeScopeLockedMutex __name__(mutex);

bool		StopThread = false;

int ResourcesHandlerThreadFunction(void* pData)
{
	auto pThis = (FeModuleRenderResourcesHandler*)pData;

	while (!StopThread)
	{
		pThis->ProcessThreadedTexturesLoading(StopThread);
		SDL_Delay(100);
	}
	
	return 0;
}
uint32 FeModuleRenderResourcesHandler::ProcessThreadedTexturesLoading(bool& bThreadSopped)
{
	TexturesLoadingMap* pTexturesToLoad = NULL;
	{
		SCOPELOCK(TexturesLoadingMutex); // <------ Lock Mutex
		pTexturesToLoad = new TexturesLoadingMap(TexturesLoading);
	}

	TexturesLoadingMap& texturesToLoad = *pTexturesToLoad;

	for (TexturesLoadingMapIt it = texturesToLoad.begin(); it != texturesToLoad.end(); ++it)
	{
		if (bThreadSopped)
			break;

		FeRenderLoadingTexture& texture = it->second;

		ID3D11Device* pD3DDevice = FeModuleRendering::GetDevice().GetD3DDevice();
		ID3D11DeviceContext* pD3DContext = FeModuleRendering::GetDevice().GetLoadingThreadContext();

		HRESULT hr;
		D3DX11_IMAGE_LOAD_INFO loadinfos;
		D3DX11_IMAGE_INFO imgInfos;
		ZeroMemory(&loadinfos, sizeof(D3DX11_IMAGE_LOAD_INFO));

#if USE_DDS_IF_EXISTS
		FeFile ddsPath;
		FeFileSystem::GetFullPathChangeExtension(ddsPath, texture.Path, "dds");
		const char* szPath = FeFileSystem::FileExists(ddsPath) ? ddsPath.Path : texture.Path;
#else
		const char* szPath = texture.Path;
#endif
		D3DX11GetImageInfoFromFile(szPath, NULL, &imgInfos, &hr);

		uint32 iForcedFormat = DXGI_FORMAT_BC1_UNORM;//DXGI_FORMAT_B8G8R8A8_UNORM
		uint32 iResize = 2;
		
		loadinfos.Width = imgInfos.Width / iResize;
		loadinfos.Height = imgInfos.Height / iResize;

		loadinfos.Depth = imgInfos.Depth;
		loadinfos.FirstMipLevel = 0;
		loadinfos.MipLevels = 1;
		loadinfos.Usage = D3D11_USAGE_DEFAULT;//D3D11_USAGE_DYNAMIC
		loadinfos.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		loadinfos.CpuAccessFlags = 0;//D3D11_CPU_ACCESS_WRITE
		loadinfos.MiscFlags = 0;
		loadinfos.Format = iForcedFormat ? (DXGI_FORMAT)iForcedFormat : imgInfos.Format;
		loadinfos.Filter = D3DX11_FILTER_LINEAR;
		loadinfos.MipFilter = D3DX11_FILTER_LINEAR;
		loadinfos.pSrcInfo = &imgInfos;

		texture.SizeInMemory = ComputeTextureSizeInMemoryFromFormat(loadinfos.Width, loadinfos.Height, loadinfos.Format, true);
		
		if (texture.SizeInMemory + TexturePoolAllocated > TexturePoolLimit)
		{
			break;
		}

		texture.WasConverted = loadinfos.Format != imgInfos.Format;

		D3DX11CreateTextureFromFile(pD3DDevice, texture.Path, &loadinfos, NULL, &texture.Resource, &hr);

		if (SUCCEEDED(hr))
		{
			hr = pD3DDevice->CreateShaderResourceView(texture.Resource, NULL, &texture.SRV);
			texture.LoadingState = FeETextureLoadingState::Loaded;
		}
		
		if (FAILED(hr))
		{
			FE_ASSERT(false, "texture  loading failed");
			texture.LoadingState = FeETextureLoadingState::LoadFailed;
		}

		{
			SCOPELOCK(TexturesLoadingMutex); // <------ Lock Mutex
			TexturesLoading.erase(it->first);
		}

		{
			SCOPELOCK(TexturesLoadedMutex); // <------ Lock Mutex
			TexturesLoaded[it->first] = texture;
		}
	}

	delete pTexturesToLoad;

	return FeEReturnCode::Success;
}
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
		iTextureSize = ((iWidth*iHeight) / 2)*(iPixelBitSize / 8);
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

		iTextureSize = (iWidth*iHeight)*(iPixelBitSize / 8);
	}

	return iTextureSize;
}
void FeModuleRenderResourcesHandler::ComputeDebugInfos(FeModuleRenderResourcesHandlerDebugInfos& infos)
{
	infos.LoadedTexturesCount = 0;
	infos.LoadedTexturesCountSizeInMemory = 0;
	infos.TexturesPoolSize = TexturePoolLimit;
	
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
uint32 FeModuleRenderResourcesHandler::Load(const FeModuleInit*)
{
	TexturePoolLimit = 256*(1024*1024);
	TexturePoolAllocated = 0;

	TexturesLoadingMutex = SDL_CreateMutex();
	TexturesLoadedMutex = SDL_CreateMutex();

	LoadingThread = SDL_CreateThread(ResourcesHandlerThreadFunction, "ModuleRenderResourcesHandler", this);
	
	return FeEReturnCode::Success;
}
uint32 FeModuleRenderResourcesHandler::Unload()
{
	StopThread = true;
	int iReturned;
	SDL_WaitThread(LoadingThread, &iReturned);

	SDL_DestroyMutex(TexturesLoadingMutex);
	SDL_DestroyMutex(TexturesLoadedMutex);

	return FeEReturnCode::Success;
}
uint32 FeModuleRenderResourcesHandler::Update(const FeDt& fDt)
{
	{
		SCOPELOCK(TexturesLoadedMutex); // <------ Lock Mutex

		for (TexturesLoadingMapIt it = TexturesLoaded.begin(); it != TexturesLoaded.end(); ++it)
		{
			FeRenderLoadingTexture& texture = it->second;

			FeRenderTexture loadedTexture;
			loadedTexture.LoadingState	= texture.LoadingState;
			loadedTexture.Resource		= texture.Resource;
			loadedTexture.SizeInMemory	= texture.SizeInMemory;
			loadedTexture.SRV			= texture.SRV;
			
			Textures[it->first] = loadedTexture;

			TexturePoolAllocated += loadedTexture.SizeInMemory;

#if SAVE_CONVERTED_TO_DDS
			if (texture.WasConverted)
				TexturesToSave[it->first] = texture;
#endif
		}

		TexturesLoaded.clear();
	}
	ID3D11DeviceContext* pD3DContext = FeModuleRendering::GetDevice().GetImmediateContext();

	for (TexturesLoadingMapIt it = TexturesToSave.begin(); it != TexturesToSave.end(); ++it)
	{
		FeRenderLoadingTexture& texture = it->second;

		FeFile savedFile;
		FeFileSystem::GetFullPathChangeExtension(savedFile, texture.Path, "dds");
		HRESULT hr = D3DX11SaveTextureToFile(pD3DContext, texture.Resource, D3DX11_IFF_DDS, savedFile.Path);
		if (FAILED(hr))
		{
			FE_ASSERT(false, "converted texture save failed");
		}
		else
		{
			FE_LOG("Save converted texture %s", texture.Path);
		}
		TexturesToSave.erase(it);
		break; //  process one file per frame
	}
	return FeEReturnCode::Success;
}
const FeRenderTexture* FeModuleRenderResourcesHandler::GetTexture(const FeRenderTextureId& textureId) const
{
	TexturesMap::const_iterator it = Textures.find(textureId);
	return (it != Textures.end()) ? &it->second : NULL;
}
uint32 FeModuleRenderResourcesHandler::LoadTexture(const char* szTexturePath, FeRenderTextureId* pTextureId)
{
	char szPath[COMMON_PATH_SIZE];
	sprintf_s(szPath, szTexturePath);
	FeStringTools::ToLower(szPath);

	*pTextureId = FeStringTools::GenerateUIntIdFromString(szPath);
	FeRenderTextureId iTexId = *pTextureId;

	if (!GetTexture(iTexId))
	{
		SCOPELOCK(TexturesLoadingMutex); // <------ Lock Mutex

		TexturesLoadingMap::const_iterator it = TexturesLoading.find(iTexId);
		
		if (it == TexturesLoading.end())
		{
			// todo: make all of it asynchone & thread safe
			TexturesLoading[iTexId] = FeRenderLoadingTexture(); // add texture to map

			FeRenderLoadingTexture& texture = TexturesLoading[iTexId];
			ZeroMemory(&texture, sizeof(FeRenderLoadingTexture));
			sprintf_s(texture.Path, COMMON_PATH_SIZE, szPath);
			texture.LoadingState = FeETextureLoadingState::Idle;
		}
	}
	return FeEReturnCode::Success;
}
uint32 FeModuleRenderResourcesHandler::UnloadTexture(const FeRenderTextureId&)
{
	// todo
	return FeEReturnCode::Success;
}
