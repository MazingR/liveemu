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

extern "C"
{
	int z_verbose = 0;

	void z_error(/* should be const */char* message)
	{
		FE_LOG(message);
	}
}

#include <ft2build.h>
#include <freetype/freetype.h>

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
	{
		SCOPELOCK(ResourcesLoadingThreadMutex); // <------ Lock Mutex

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
				iRet = LoadTexture(resource, (FeRenderTexture*)resource.Interface->GetData());
			} break;
			case FeEResourceType::Font:
			{
				iRet = LoadFont(resource, (FeRenderFont*)resource.Interface->GetData());
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
	} // <----- ResourcesLoadingThreadMutex

	return FeEReturnCode::Success;
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
	ResourcesLoadingThreadMutex = SDL_CreateMutex();

	LoadingThread = SDL_CreateThread(ResourcesHandlerThreadFunction, "ModuleRenderResourcesHandler", this);

	// Initialize freetype
	auto error = FT_Init_FreeType(&FtLibrary);
	
	if (error)
	{
		return FeEReturnCode::Failed;
	}
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
	ID3D11DeviceContext* pD3DContext = FeModuleRendering::GetDevice().GetImmediateContext();

	{
		SCOPELOCK(ResourcesLoadedMutex); // <------ Lock Mutex

		for (ResourcesLoadingMapIt it = ResourcesLoaded.begin(); it != ResourcesLoaded.end(); ++it)
		{
			FeRenderLoadingResource& resource = it->second;

			if (resource.Type == FeEResourceType::Font)
				PostLoadFont(resource, (FeRenderFont*)resource.Interface->GetData());

			FeRenderResource& loadedResource = Resources[it->first];

			loadedResource.LoadingState	= resource.LoadingState;
			loadedResource.SizeInMemory	= resource.SizeInMemory;

			ResourcePoolAllocated += loadedResource.SizeInMemory;
#if SAVE_CREATED_RESOURCE
			if (resource.RuntimeCreated && resource.LoadingState == FeEResourceLoadingState::Loaded && resource.Type == FeEResourceType::Texture)
#else
			if (false)
#endif
			{
				ResourcesToSave[it->first] = resource;
			}
		}

		ResourcesLoaded.clear();
	}

	for (ResourcesLoadingMapIt it = ResourcesToSave.begin(); it != ResourcesToSave.end(); ++it)
	{
		FeRenderLoadingResource& resource = it->second;

		FePath savedFile;
		FeFileTools::GetFullPathChangeExtension(savedFile, resource.Path.Value, "dds");

		FeRenderTexture* pTextureData = (FeRenderTexture*)resource.Interface->GetData();
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
	if (resource.Id == 0)
	{
		resource.Id = FeStringTools::GenerateUIntIdFromString(resource.Path.Value);
	}

	if (!GetResource(resource.Id))
	{
		Resources[resource.Id] = FeRenderResource();
		FeRenderResource& newResource = Resources[resource.Id];
		newResource.LoadingState = FeEResourceLoadingState::Idle;
		newResource.Type = resource.Type;
		newResource.Interface = resource.Interface;

		{
			SCOPELOCK(ResourcesLoadingMutex); // <------ Lock Mutex
			
			if (newResource.Interface == NULL)
			{
				switch (resource.Type)
				{
				case FeEResourceType::Texture: newResource.Interface = FeCreateRenderResourceInterface<FeRenderTexture>();	break;
				case FeEResourceType::Font: newResource.Interface = FeCreateRenderResourceInterface<FeRenderFont>();	break;
				default:
				{
					FE_ASSERT(false, "Unknown render resource type !");
				}
				}
			}
			
			ResourcesLoading[resource.Id] = resource;
			ResourcesLoading[resource.Id].Interface = newResource.Interface;
		} // <------ Unlock Mutex
	}
	return FeEReturnCode::Success;
}
uint32 FeModuleRenderResourcesHandler::LoadFont(FeRenderLoadingResource& resource, FeRenderFont* pFont)
{
	bool bLoadingFailed = false;
	pFont->MapTmpData = NULL;
	pFont->MapDepthPitch = 0;
	
	char szFullPath[COMMON_PATH_SIZE];
	sprintf_s(szFullPath, "%s%s", FeFileTools::GetRootDir().Value, resource.Path.Value);
	
	auto error = FT_New_Face(FtLibrary, szFullPath, 0, &pFont->FtFontFace);
	FT_FaceRec_* face = pFont->FtFontFace;

	if (error == FT_Err_Unknown_File_Format)
	{
		FE_LOG("the font file could be opened and read, but it appears that its font format is unsupported");
		return FeEReturnCode::Failed;
	}
	else if (error)
	{
		FE_LOG("that the font file could not be opened or read, or that it is broken..");
		return FeEReturnCode::Failed;
	}

	uint32 iCharSize = pFont->Size;
	uint32 iMapWidth	= 0;
	uint32 iMapHeight	= 0;
	uint32 iCharInterval = 4;
	const char szTemplateFontContent[] = {
"abcdefghijklmnopqrstuvwxyz\
ABCDEFGHIJKLMNOPQRSTUVWXYZ\
123456789\
àéêâùçèûôîµ\
,;:!?./§%*€$\\&#'{([-|_@)]})=\
"
/*
*/
	};
	const uint32 iMaxCharCount = sizeof(szTemplateFontContent);

	char szFontContent[iMaxCharCount];

	uint32 iCharCount = 0;
	
	// Compute actual charCount
	for (size_t iChar = 0; iChar < iMaxCharCount; ++iChar)
	{
		/* retrieve glyph index from character code */
		if (FT_Get_Char_Index(face, szTemplateFontContent[iChar]) == 0)
			continue;

		szFontContent[iCharCount++] = szTemplateFontContent[iChar];
	}
	const uint32 iCharsPerLine = 16;

	while (iMapWidth < (iCharsPerLine*(iCharSize + iCharInterval)))
		iMapWidth += 2;

	uint32 iCharPerLine = ceil(iMapWidth / (float)(iCharSize + iCharInterval));
	uint32 iLinesCount = ceil(iCharCount / (float)iCharPerLine);

	while (iMapHeight <(iCharSize*iLinesCount))
		iMapHeight += 2;

	pFont->MapSize[0] = iMapWidth;
	pFont->MapSize[1] = 0;

	uint32 iXOffset = 0;
	uint32 iYOffset = 0;
	uint32 iDepthPitch = iMapWidth*iMapHeight;
	
	error = FT_Set_Char_Size(
		face,				/* handle to face object           */
		0,					/* char_width in 1/64th of points  */
		iCharSize * 64,	/* char_height in 1/64th of points */
		iMapWidth,			/* horizontal device resolution    */
		iMapHeight);		/* vertical device resolution      */
	
	error = FT_Set_Pixel_Sizes(
		face,			/* handle to face object */
		0,				/* pixel_width           */
		iCharSize);	/* pixel_height          */
	
	FT_GlyphSlot  slot = face->glyph;  /* a small shortcut */

	pFont->MapTmpData = FE_ALLOCATE(iDepthPitch, RENDERER_HEAP);
	pFont->MapDepthPitch = iDepthPitch;

	memset(pFont->MapTmpData, 0, iDepthPitch);

	for (size_t iChar = 0; iChar < iCharCount; ++iChar)
	{
		FT_UInt  glyph_index;
	
		/* retrieve glyph index from character code */
		glyph_index = FT_Get_Char_Index(face, szFontContent[iChar]);
	
		/* load glyph image into the slot (erase previous one) */
		error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
		
		if (error)
			continue;  /* ignore errors */
	
		/* convert to an anti-aliased bitmap */
		error = FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL);

		if (error)
			continue;

		if (!slot->bitmap.buffer)
			continue;

		char* pOutput = (char*)pFont->MapTmpData;

		// Copy char texture to font atlas map
		for (uint32 iY = 0; iY < slot->bitmap.rows; ++iY)
		{
			uint32 inputBufferIdx = slot->bitmap.pitch*iY;// +slot->bitmap_left;
			uint32 outputBufferIdx = iMapWidth*(iY + iYOffset + slot->advance.y) + iXOffset;
			
			if (outputBufferIdx + slot->bitmap.width > iDepthPitch)
				break;

			memcpy_s(&pOutput[outputBufferIdx], slot->bitmap.width, &slot->bitmap.buffer[inputBufferIdx], slot->bitmap.width);
		}
		FeRenderFontChar charData;

		charData.Left		= iXOffset;
		charData.Top		= iYOffset;
		charData.OffsetLeft = slot->bitmap_left;
		charData.OffsetTop	= slot->bitmap_top;
		charData.Width		= slot->bitmap.width;
		charData.Height		= slot->bitmap.rows;

		pFont->Chars[szFontContent[iChar]] = charData;

		iXOffset += slot->bitmap.width + slot->bitmap_left + iCharInterval;

		if (iXOffset > (iMapWidth - iCharSize))
		{
			iYOffset += iCharSize;
			iXOffset = 0;

			pFont->MapSize[1] += iCharSize;
		}
	}
	pFont->MapSize[1] += iCharSize;

	return FeEReturnCode::Success;
}
uint32 FeModuleRenderResourcesHandler::PostLoadFont(FeRenderLoadingResource& resource, FeRenderFont* pFont)
{
	ID3D11DeviceContext* pD3DContext = FeModuleRendering::GetDevice().GetImmediateContext();
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

	desc.Width = pFont->MapSize[0];
	desc.Height = pFont->MapSize[1];
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ID3D11Device* pD3DDevice = FeModuleRendering::GetDevice().GetD3DDevice();
	ID3D11Texture2D *pTexture = NULL;

	auto hr = pD3DDevice->CreateTexture2D(&desc, NULL, &pTexture);

	if (FAILED(hr))
		return FeEReturnCode::Failed;

	pFont->Texture.D3DResource = reinterpret_cast<ID3D11Resource*>(pTexture);

	D3D11_MAPPED_SUBRESOURCE textureMap;
	hr = pD3DContext->Map(pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &textureMap);

	if (FAILED(hr))
		return FeEReturnCode::Failed;

	//memset(textureMap.pData, 0xFF, textureMap.DepthPitch);
	char* pOutput = (char*)textureMap.pData;
	char* pInput = (char*)pFont->MapTmpData;

	uint32 iFontMapWidth = pFont->MapSize[0];
	uint32 iFontMapHeight = pFont->MapSize[1];

	for (uint32 i = 0; i < iFontMapHeight; ++i)
		memcpy_s(pOutput + (i*textureMap.RowPitch), iFontMapWidth, pInput + (i*iFontMapWidth), iFontMapWidth);

	pD3DContext->Unmap(pTexture, 0);

	hr = pD3DDevice->CreateShaderResourceView(pTexture, NULL, &pFont->Texture.D3DSRV);
	if (FAILED(hr))
		return FeEReturnCode::Failed;

	FE_FREE(pFont->MapTmpData, RENDERER_HEAP);

	return FeEReturnCode::Success;
}
uint32 FeModuleRenderResourcesHandler::LoadTexture(FeRenderLoadingResource& resource, FeRenderTexture* pTextureData)
{
	ID3D11Device* pD3DDevice = FeModuleRendering::GetDevice().GetD3DDevice();

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
void FeModuleRenderResourcesHandler::UnloadResources()
{
	{
		SCOPELOCK(ResourcesLoadingThreadMutex); // <------ Lock Mutex
		for (auto& resource : Resources)
		{
			if (resource.second.Interface)
			{
				resource.second.Interface->ReleaseResource();
				resource.second.Interface->Release();
			}
		}
		Resources.clear();
		ResourcesLoading.clear();
		ResourcesToSave.clear();
	}
}