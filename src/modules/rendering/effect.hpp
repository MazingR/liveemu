#pragma once

#include <commonrenderer.hpp>
#include <common/serializable.hpp>

// forward declares
struct FeRenderGeometryInstance;
struct FeRenderViewport;
struct FeRenderGeometryInstance;
class FeModuleRenderResourcesHandler;

class FeRenderEffect : public FeSerializable
{
public:
	void BeginFrame(const FeRenderCamera& camera, const FeRenderViewport& viewport, float fDt);
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
		
	FeTArray<FeRenderSampler>			Samplers;

	FeRenderConstantBuffer				CBPerFrame;
	FeRenderConstantBuffer				CBPerObject;

	#define FeRenderEffect_Properties(_d)		\
		_d(FeString,					Name)	\
		_d(FePath,						File)	\
		_d(uint32,						TextureLevels)\

	FE_DECLARE_CLASS_BODY(FeRenderEffect_Properties, FeRenderEffect, FeSerializable)
};
FE_DECLARE_CLASS_BOTTOM(FeRenderEffect)