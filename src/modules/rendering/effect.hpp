#pragma once

#include <commonrenderer.hpp>
#include <common/serializable.hpp>

// forward declares
struct FeRenderGeometryInstance;
struct FeRenderViewport;
struct FeRenderGeometryInstance;
class FeModuleRenderResourcesHandler;


#define FeERenderEffectType_Values(_d)		\
		_d(FeERenderEffectType, Image)		\
		_d(FeERenderEffectType, Font)		\

FE_DECLARE_ENUM(FeERenderEffectType, FeERenderEffectType_Values)


class FeRenderEffect : public FeSerializable
{
public:
	void BeginFrame(const FeRenderCamera& camera, const FeRenderViewport& viewport, float fDt);
	void EndFrame();

	void BindGeometryInstance(const FeRenderGeometryInstance geometry, const FeModuleRenderResourcesHandler* resouresHandler);
	void Bind();
	void Release();
	uint32 CreateFromFile(const char* szFilePath);

	FeRenderEffect()
	{
		TextureLevels = 0;
		UseAlphaBlending = false;
		Type = FeERenderEffectType::Image;
	}
private:
	uint32 CompileShaderFromFile(const char* szFileName, const char* szEntryPoint, const char* szShaderModel, void** ppBlobOut);

	ID3D11VertexShader*					VertexShader;
	ID3D11PixelShader*					PixelShader;
	ID3D11InputLayout*					VertexLayout;
	ID3D11BlendState*					BlendState;

	FeTArray<FeRenderSampler>			Samplers;

	FeRenderConstantBuffer				CBPerFrame;
	FeRenderConstantBuffer				CBPerObject;

	#define FeRenderEffect_Properties(_d)					\
		_d(FeString,					Name)				\
		_d(FePath,						File)				\
		_d(uint32,						TextureLevels)		\
		_d(bool,						UseAlphaBlending)	\
		_d(FeERenderEffectType::Type,	Type)				\

	FE_DECLARE_CLASS_BODY(FeRenderEffect_Properties, FeRenderEffect, FeSerializable)
};
FE_DECLARE_CLASS_BOTTOM(FeRenderEffect)