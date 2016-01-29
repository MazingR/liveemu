#pragma once

#include <common/common.hpp>
#include <common/tarray.hpp>
#include <common/maths.hpp>
#include <map>

#define DEBUG_STRING_SIZE 2048
#define RENDERER_HEAP 1
#define RENDERER_DEFAULT_EFFECT_ID 1

// forward declares of DXSDK
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11Resource;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;
struct ID3D11Buffer;
struct ID3D11SamplerState;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;
struct ID3D11BlendState;

// forward declares of FW1 FontWrapper 
struct IFW1Factory;
struct IFW1FontWrapper;
struct  FT_FaceRec_;
typedef uint32 FeResourceId;

struct FeRenderCamera
{
	FeVector3	VecPosition;
	FeVector3	VecLookAt;
	FeVector3	VecUp;

	FeMatrix4	MatrixProjection;
	FeMatrix4	MatrixView;
};
struct FeRenderConstantBuffer
{
	ID3D11Buffer*	Buffer;
};
struct FeRenderSampler
{
	ID3D11SamplerState*	State;
};

namespace FeEDebugRenderTextMode
{
	enum Type
	{
		Rendering,
		Memory,
		Count
	};
};
namespace FeEResourceLoadingState
{
	enum Type
	{
		Idle,
		Loading,
		Loaded,
		LoadFailed
	};
}
typedef uint32 FeResourceId;

struct FeTextureLoadingQueryResult
{
	FeResourceId TextureId;
};

namespace FeEResourceType
{
	enum Type
	{
		Texture,
		Font,
		Geometry,
		Sound,
		Unknown
	};
}

class FeIRenderResourceInterface
{
public:
	virtual void CopyTo(FeIRenderResourceInterface* pCopy) = 0;
	virtual void CopyAndAllocateTo(FeIRenderResourceInterface** pCopy) = 0;
	virtual void Release()=0;
	virtual void ReleaseResource() = 0;
	
	virtual void* GetData() = 0;
};

template<typename T>
class FeRenderResourceInterface : public FeIRenderResourceInterface
{
public:
	void CopyTo(FeIRenderResourceInterface* pCopy)
	{
		memcpy_s(pCopy, sizeof(FeRenderResourceInterface<T>), this, sizeof(FeRenderResourceInterface<T>));
	}
	void CopyAndAllocateTo(FeIRenderResourceInterface** pCopy)
	{
		*pCopy = FE_NEW(FeRenderResourceInterface<T>, RENDERER_HEAP);
		CopyTo(*pCopy);
	}
	void Release()
	{
		FE_DELETE(FeRenderResourceInterface<T>, this, RENDERER_HEAP);
	}
	void ReleaseResource()
	{
		Data.Release();
	}

	void* GetData()
	{
		return &Data;
	}

	T* GetTData()
	{
		return &Data;
	}
	T Data;
};

template<typename T>
static FeRenderResourceInterface<T>* FeCreateRenderResourceInterface()
{
	return FE_NEW(FeRenderResourceInterface<T>, RENDERER_HEAP);
}

struct FeRenderTexture
{
	ID3D11Resource*					D3DResource;
	ID3D11ShaderResourceView*		D3DSRV;

	FeRenderTexture() :
		D3DResource(NULL),
		D3DSRV(NULL) {}

	void Release();
};

struct FeRenderFontChar
{
	uint32 Top;
	uint32 Left;
	uint32 OffsetTop;
	uint32 OffsetLeft;
	uint32 Width;
	uint32 Height;
};
class FeRenderFont
{
public:
	FeRenderTexture		Texture;
	FT_FaceRec_*		FtFontFace;
	void*				MapTmpData;
	uint32				MapDepthPitch;
	FeSize				MapSize;
	uint32				Size;
	uint32				Space;
	uint32				Interval;
	FePath				TrueTypeFile;

	std::map<uint8, FeRenderFontChar> Chars;

	FeRenderFont() :
		FtFontFace(NULL),
		MapTmpData(NULL) {}

	void Release();
};

struct FeRenderResource
{
	FeEResourceType::Type			Type;
	FeEResourceLoadingState::Type	LoadingState;
	uint32							SizeInMemory;
	bool							RuntimeCreated;
	FeIRenderResourceInterface*		Interface;

	FeRenderResource() : Interface(NULL), SizeInMemory(0) {}
};
typedef void(*FeResourceLoadingCallbackFunc) (FeRenderResource* pResource, void* pUserData);

struct FeResourceLoadingCallback
{
	FeResourceLoadingCallbackFunc	Functor;
	void*							UserData;
};

struct FeRenderLoadingResource : public FeRenderResource
{
	FePath						Path;
	FeResourceId				Id;
	FeResourceLoadingCallback	LoadingFinishedCallback;

	FeRenderLoadingResource() : Id(0) {}
};
struct FeRenderViewport
{
	uint32					Width;
	uint32					Height;

	ID3D11RenderTargetView* RenderTargetView;
	ID3D11DepthStencilView*	DepthStencilView;
	ID3D11Texture2D*		DepthStencil;

	uint32 CreateFromBackBuffer();

	void Bind()  const;
	void Clear()  const;
	void Unload();
};

struct FeRenderGeometryData
{
	ID3D11Buffer*	VertexBuffer;
	ID3D11Buffer*	IndexBuffer;
	uint32			Stride;
	uint32			IndexCount;

	void Release();
};

struct FeGeometryTransform
{
	FeMatrix4	Matrix;
	static const FeMatrix4& IdentityMatrix();
};
struct FeRenderGeometryInstance
{
	FeResourceId					Geometry;
	FeResourceId					Effect;
	FeGeometryTransform				Transform;
	FeVector4						UserData;
	FeTArray<FeResourceId>			Textures;

	FeRenderGeometryInstance()
	{
		Textures.SetHeapId(RENDERER_HEAP);
		Textures.Reserve(1);
		Textures.Clear();
	}
};
namespace FeEGemetryDataType
{
	enum Type
	{
		Quad,
		Count
	};
};