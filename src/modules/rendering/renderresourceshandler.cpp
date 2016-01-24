#include <renderresourceshandler.hpp>
#include <modulerenderer.hpp>

#include <common/memorymanager.hpp>
#include <common/filesystem.hpp>
#include <common/string.hpp>

#include <d3dx11include.hpp>
#include <SDL.h>

#define USE_DDS_IF_EXISTS 1
#define SAVE_CREATED_RESOURCE 1
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
		pThis->ProcessThreadedResourcesLoading(StopThread);
		SDL_Delay(100);
	}
	
	return 0;
}

uint32 FeModuleRenderResourcesHandler::ProcessThreadedResourcesLoading(bool& bThreadSopped)
{
	ResourcesLoadingMap* pResourcesToLoad = NULL;
	{
		SCOPELOCK(ResourcesLoadingMutex); // <------ Lock Mutex
		pResourcesToLoad = new ResourcesLoadingMap(ResourcesLoading);
	}

	ResourcesLoadingMap& resourcesToLoad = *pResourcesToLoad;
	
	for (auto& idedResource : resourcesToLoad)
	{
		if (bThreadSopped)
			break;

		FeRenderLoadingResource& resource = idedResource.second;
		uint32 iRet = FeEReturnCode::Failed;

		switch (resource.Type)
		{
			case FeEResourceType::Texture:
			{
				iRet = CreateTexture(resource, (FeRenderTextureData*)resource.Interface->GetData());
			} break;
		}
		
		resource.LoadingState = FE_FAILED(iRet) ? FeEResourceLoadingState::LoadFailed : FeEResourceLoadingState::Loaded;

		{
			SCOPELOCK(ResourcesLoadingMutex); // <------ Lock Mutex
			ResourcesLoading.erase(idedResource.first);
		}

		{
			SCOPELOCK(ResourcesLoadedMutex); // <------ Lock Mutex
			ResourcesLoaded[idedResource.first] = resource;
		}
	}

	delete pResourcesToLoad;

	return FeEReturnCode::Success;
}
uint32 FeModuleRenderResourcesHandler::ComputeResourceSizeInMemoryFromFormat(uint32 _iWidth, uint32 _iHeight, uint32 iResourceFormat, bool bHasAlpha)
{
	uint32 iWidth = _iWidth;
	uint32 iHeight = _iHeight;

	uint32 iResourceSize = 0;
	DXGI_FORMAT iFormat = (DXGI_FORMAT)iResourceFormat;
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

	if (iPixelBitSize != 0) // resource format is compressed
	{
		uint32 iSizeMultiple = 256;
		iWidth = (uint32)ceil((float)(_iWidth / (float)iSizeMultiple)) * iSizeMultiple;
		iHeight = (uint32)ceil((float)(_iHeight / (float)iSizeMultiple)) * iSizeMultiple;
		iResourceSize = ((iWidth*iHeight) / 2)*(iPixelBitSize / 8);
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

		iResourceSize = (iWidth*iHeight)*(iPixelBitSize / 8);
	}

	return iResourceSize;
}
void FeModuleRenderResourcesHandler::ComputeDebugInfos(FeModuleRenderResourcesHandlerDebugInfos& infos)
{
	infos.LoadedResourcesCount = 0;
	infos.LoadedResourcesCountSizeInMemory = 0;
	infos.ResourcesPoolSize = ResourcePoolLimit;
	
	for (ResourcesMapIt it = Resources.begin(); it != Resources.end(); ++it)
	{
		FeRenderResource& resource = it->second;

		if (resource.LoadingState == FeEResourceLoadingState::Loaded)
		{
			infos.LoadedResourcesCount++;
			infos.LoadedResourcesCountSizeInMemory += resource.SizeInMemory;
		}
	}
}
uint32 FeModuleRenderResourcesHandler::Load(const FeModuleInit*)
{
	ResourcePoolLimit = 256*(1024*1024);
	ResourcePoolAllocated = 0;

	ResourcesLoadingMutex = SDL_CreateMutex();
	ResourcesLoadedMutex = SDL_CreateMutex();

	LoadingThread = SDL_CreateThread(ResourcesHandlerThreadFunction, "ModuleRenderResourcesHandler", this);
	
	return FeEReturnCode::Success;
}
uint32 FeModuleRenderResourcesHandler::Unload()
{
	StopThread = true;
	int iReturned;
	SDL_WaitThread(LoadingThread, &iReturned);

	SDL_DestroyMutex(ResourcesLoadingMutex);
	SDL_DestroyMutex(ResourcesLoadedMutex);

	return FeEReturnCode::Success;
}
uint32 FeModuleRenderResourcesHandler::Update(const FeDt& fDt)
{
	{
		SCOPELOCK(ResourcesLoadedMutex); // <------ Lock Mutex

		for (ResourcesLoadingMapIt it = ResourcesLoaded.begin(); it != ResourcesLoaded.end(); ++it)
		{
			FeRenderLoadingResource& resource = it->second;

			FeRenderResource loadedResource;
			loadedResource.LoadingState	= resource.LoadingState;
			loadedResource.SizeInMemory	= resource.SizeInMemory;
			
			resource.Interface->CopyAndAllocateTo(&loadedResource.Interface);
			
			Resources[it->first] = loadedResource;
			ResourcePoolAllocated += loadedResource.SizeInMemory;

#if SAVE_CREATED_RESOURCE
			if (resource.RuntimeCreated && resource.LoadingState == FeEResourceLoadingState::Loaded)
#else
			if (false)
#endif
			{
				ResourcesToSave[it->first] = resource;
			}
			else
			{
				resource.Interface->Release();
			}
		}

		ResourcesLoaded.clear();
	}
	ID3D11DeviceContext* pD3DContext = FeModuleRendering::GetDevice().GetImmediateContext();

	for (ResourcesLoadingMapIt it = ResourcesToSave.begin(); it != ResourcesToSave.end(); ++it)
	{
		FeRenderLoadingResource& resource = it->second;

		FePath savedFile;
		FeFileTools::GetFullPathChangeExtension(savedFile, resource.Path.Value, "dds");

		FeRenderTextureData* pTextureData = (FeRenderTextureData*)resource.Interface->GetData();
		char szFullPath[COMMON_PATH_SIZE];
		sprintf_s(szFullPath, "%s%s", FeFileTools::GetRootDir().Value, savedFile.Value);

		HRESULT hr = D3DX11SaveTextureToFile(pD3DContext, pTextureData->D3DResource, D3DX11_IFF_DDS, szFullPath);

		if (FAILED(hr))
		{
			FE_ASSERT(false, "converted resource save failed");
		}
		else
		{
			FE_LOG("Save converted resource %s", resource.Path.Value);
		}

		resource.Interface->Release();
		ResourcesToSave.erase(it);

		break; //  process one file per frame
	}
	return FeEReturnCode::Success;
}
const FeRenderResource* FeModuleRenderResourcesHandler::GetResource(const FeResourceId& resourceId) const
{
	ResourcesMap::const_iterator it = Resources.find(resourceId);
	return (it != Resources.end()) ? &it->second : NULL;
}
uint32 FeModuleRenderResourcesHandler::UnloadResource(const FeResourceId&)
{
	// todo
	return FeEReturnCode::Success;
}
uint32 FeModuleRenderResourcesHandler::LoadResource(FeRenderLoadingResource& resource)
{
	FeStringTools::ToLower(resource.Path.Value);
	resource.Id = FeStringTools::GenerateUIntIdFromString(resource.Path.Value);

	if (!GetResource(resource.Id))
	{
		SCOPELOCK(ResourcesLoadingMutex); // <------ Lock Mutex

		ResourcesLoadingMap::const_iterator it = ResourcesLoading.find(resource.Id);

		if (it == ResourcesLoading.end())
		{
			resource.LoadingState = FeEResourceLoadingState::Idle;
			switch (resource.Type)
			{
			case FeEResourceType::Texture: resource.Interface = FeCreateRenderResourceInterface<FeRenderTextureData>();	break;
			case FeEResourceType::Font: resource.Interface = FeCreateRenderResourceInterface<FeRenderFontData>();	break;
			default:
			{
				FE_ASSERT(false, "Unknown render resource type !");
			}
			}
			ResourcesLoading[resource.Id] = resource;
		}
	}

	return FeEReturnCode::Success;
}
uint32 FeModuleRenderResourcesHandler::CreateTexture(FeRenderLoadingResource& resource, FeRenderTextureData* pTextureData)
{
	ID3D11Device* pD3DDevice = FeModuleRendering::GetDevice().GetD3DDevice();
	ID3D11DeviceContext* pD3DContext = FeModuleRendering::GetDevice().GetLoadingThreadContext();

	HRESULT hr;
	D3DX11_IMAGE_LOAD_INFO loadinfos;
	D3DX11_IMAGE_INFO imgInfos;
	ZeroMemory(&loadinfos, sizeof(D3DX11_IMAGE_LOAD_INFO));
	
	FePath fullPath;
	sprintf_s(fullPath.Value, "%s%s", FeFileTools::GetRootDir().Value, resource.Path.Value);

#if USE_DDS_IF_EXISTS
	FePath ddsPath;
	FeFileTools::GetFullPathChangeExtension(ddsPath, fullPath.Value, "dds");
	
	if (FeFileTools::FileExists(ddsPath))
		fullPath = ddsPath;
#endif
	D3DX11GetImageInfoFromFile(fullPath.Value, NULL, &imgInfos, &hr);

	uint32 iForcedFormat = DXGI_FORMAT_BC3_UNORM;//DXGI_FORMAT_B8G8R8A8_UNORM DXGI_FORMAT_BC3_UNORM
	uint32 iResize = 1;

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

	resource.SizeInMemory = ComputeResourceSizeInMemoryFromFormat(loadinfos.Width, loadinfos.Height, loadinfos.Format, true);

	if (resource.SizeInMemory + ResourcePoolAllocated <= ResourcePoolLimit)
	{
		resource.RuntimeCreated = loadinfos.Format != imgInfos.Format;

		D3DX11CreateTextureFromFile(pD3DDevice, fullPath.Value, &loadinfos, NULL, &pTextureData->D3DResource, &hr);

		if (SUCCEEDED(hr))
		{
			hr = pD3DDevice->CreateShaderResourceView(pTextureData->D3DResource, NULL, &pTextureData->D3DSRV);
		}

		return SUCCEEDED(hr) ? FeEReturnCode::Success : FeEReturnCode::Failed;
	}
	else
	{
		return FeEReturnCode::Failed;
	}
}