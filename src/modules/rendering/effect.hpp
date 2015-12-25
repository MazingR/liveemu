#pragma once

#include <common/common.hpp>

#pragma warning( disable : 4005)
#include <d3d11.h>
#include <map>

namespace FeRendering
{
	struct FeRenderGeometryInstance;
	struct FeRenderViewport;
	struct FeRenderGeometryInstance;
	class FeModuleRenderResourcesHandler;

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

	class FeRenderEffect
	{
	public:
		void BeginFrame(const FeRenderCamera& camera, const FeRenderViewport& viewport);
		void EndFrame();

		void BindGeometryInstance(const FeRenderGeometryInstance geometry, const FeModuleRenderResourcesHandler* resouresHandler);
		void Bind();
		void Release();
		uint32 CreateFromFile(const char* szFilePath);
	private:
		uint32 CompileShaderFromFile(const char* szFileName, const char* szEntryPoint, const char* szShaderModel, void** ppBlobOut);

		ID3D11VertexShader*					VertexShader;
		ID3D11PixelShader*					PixelShader;
		ID3D11InputLayout*					VertexLayout;
		
		FeCommon::FeTArray<FeRenderSampler>	Samplers;

		FeRenderConstantBuffer				CBPerFrame;
		FeRenderConstantBuffer				CBPerObject;
	};
}