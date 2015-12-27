#include <modulerenderresourceshandler.hpp>
#include <modulerenderer.hpp>

#include <common/memorymanager.hpp>
#include <common/string.hpp>

#include <d3dx11include.hpp>

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
		FeStringTools::ToLower(szPath);

		*pTextureId = FeStringTools::GenerateUIntIdFromString(szPath);
		
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
} // namespace FeRendering
