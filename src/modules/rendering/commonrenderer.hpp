#pragma once

#include <common/common.hpp>
#include <common/tarray.hpp>
#include <common/maths.hpp>

#define DEBUG_STRING_SIZE 1024
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

// forward declares of FW1 FontWrapper 
struct IFW1Factory;
struct IFW1FontWrapper;

typedef uint32 FeRenderGeometryId;
typedef uint32 FeRenderEffectId;
typedef uint32 FeRenderTextureId;
typedef uint32 FeShaderId;

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
namespace FeETextureLoadingState
{
	enum Type
	{
		Idle,
		Loading,
		Loaded,
		LoadFailed
	};
}
typedef uint32 FeRenderTextureId;

struct FeTextureLoadingQueryResult
{
	FeRenderTextureId TextureId;
};

struct FeRenderTexture
{
	FeETextureLoadingState::Type	LoadingState;
	ID3D11Resource*					Resource;
	ID3D11ShaderResourceView*		SRV;
	uint32							SizeInMemory;
};
struct FeRenderLoadingTexture : public FeRenderTexture
{
	char Path[COMMON_PATH_SIZE];
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
	FeRenderGeometryId						Geometry;
	FeRenderEffectId						Effect;
	FeTArray<FeRenderTextureId>	Textures;
	FeGeometryTransform						Transform;

	FeRenderGeometryInstance()
	{
		Textures.Reserve(1);
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