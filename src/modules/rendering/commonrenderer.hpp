#pragma once

#include <common/common.hpp>
#include <common/tarray.hpp>
#include <common/maths.hpp>
#include <map>

#define DEBUG_STRING_SIZE 2048
#define FE_HEAPID_RENDERER 3
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
		Saving,
		LoadFailed,
		Count
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
		RenderTargetTexture,
		Font,
		Geometry,
		Sound,
		Unknown
	};
}

struct FeRenderResource
{
	FeEResourceLoadingState::Type	LoadingState;
	uint32							SizeInMemory;
	bool							RuntimeCreated;

	FeRenderResource() : 
		SizeInMemory(0)
	{}

	virtual void Release() = 0;
	virtual ID3D11Resource* GetD3DResource(uint32 iIdx) const = 0;
	virtual ID3D11ShaderResourceView* GetD3DSRV(uint32 iIdx) const = 0;

};
typedef void(*FeResourceLoadingCallbackFunc) (FeRenderResource* pResource, void* pUserData);

struct FeRenderTexture : public FeRenderResource
{
	ID3D11Resource*					D3DResource;
	ID3D11ShaderResourceView*		D3DSRV;

	FeRenderTexture() :
		D3DResource(nullptr),
		D3DSRV(nullptr)
	{}

	void Release();
	ID3D11Resource* GetD3DResource(uint32 iIdx) const;
	ID3D11ShaderResourceView* GetD3DSRV(uint32 iIdx) const;
};

class FeRenderTargetTexture : public FeRenderResource
{
public:
	FeRenderTexture		Texture;
	uint32				Width;
	uint32				Height;
	uint32				Channels;

	FeRenderTargetTexture()
	{}

	void Release();
	ID3D11Resource* GetD3DResource(uint32 iIdx) const;
	ID3D11ShaderResourceView* GetD3DSRV(uint32 iIdx) const;
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

class FeRenderFont : public FeRenderResource
{
public:
	FeRenderTexture		Texture;
	FT_FaceRec_*		FtFontFace;
	void*				MapTmpData;
	uint32				MapDepthPitch;
	FeSize				MapSize;
	uint32				Size;
	uint32				Space;
	uint32				LineSpace;
	uint32				Interval;
	FePath				TrueTypeFile;

	std::map<uint8, FeRenderFontChar> Chars;

	FeRenderFont() :
		FtFontFace(nullptr),
		MapTmpData(nullptr)
	{}

	void Release();
	ID3D11Resource* GetD3DResource(uint32 iIdx) const;
	ID3D11ShaderResourceView* GetD3DSRV(uint32 iIdx) const;
};

struct FeResourceLoadingCallback
{
	FeResourceLoadingCallbackFunc	Functor;
	void*							UserData;
};

class FeRenderLoadingResource
{
private:

	template<typename T>
	FeRenderResource* AllocateResource()
	{
		return FE_NEW(T, FE_HEAPID_RENDERER);
	}
public:
	FePath						Path;
	FeResourceId				Id;
	FeResourceLoadingCallback	LoadingFinishedCallback;
	FeRenderResource*			Resource;
	FeEResourceType::Type		Type;

	FeRenderLoadingResource() : 
		Id(0),
		Resource(nullptr)
	{}

	void CreateResource()
	{
		FE_ASSERT(Resource == nullptr, "resource already created !");

		switch (Type)
		{
		case FeEResourceType::Font:					Resource = AllocateResource<FeRenderFont>();	break;
		case FeEResourceType::Texture:				Resource = AllocateResource<FeRenderTexture>();	break;
		case FeEResourceType::RenderTargetTexture:	Resource = AllocateResource<FeRenderTargetTexture>();	break;
		default:
			FE_ASSERT(false, "Uknown resource type!");
		}
	}
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
	FeResourceId					Textures[2];
	void*							Owner;

	void Reset()
	{
		Geometry	= 0;
		Effect		= 0;
		Owner		= 0;
		
		Textures[0] = 0;
		Textures[1] = 0;
		
		Transform.Matrix = FeGeometryTransform::IdentityMatrix();
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