#include <renderresourceshandler.hpp>
#include <modulerenderer.hpp>

#include <common/memorymanager.hpp>
#include <common/filesystem.hpp>
#include <common/string.hpp>

#include <d3dx11include.hpp>
#include <SDL.h>


uint32 ComputeResourceSizeInMemoryFromFormat(uint32 _iWidth, uint32 _iHeight, uint32 iResourceFormat, bool bHasAlpha)
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
