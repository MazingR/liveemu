#pragma once

#include <commonrenderer.hpp>
#include <map>

// forward declares
struct FeRenderGeometryInstance;
struct FeRenderViewport;
struct FeRenderGeometryInstance;
class FeModuleRenderResourcesHandler;

class FeRenderEffect
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
		
	FeTArray<FeRenderSampler>	Samplers;

	FeRenderConstantBuffer				CBPerFrame;
	FeRenderConstantBuffer				CBPerObject;
};