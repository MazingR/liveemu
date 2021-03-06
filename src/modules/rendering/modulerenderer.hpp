#pragma once

#include <commonrenderer.hpp>
#include <common/application.hpp>

#include <geometry.hpp>
#include <effect.hpp>
#include <device.hpp>

struct FeRenderDebugInfos
{
	uint32	FrameDrawCallsCount;
	uint32	FrameBindEffectCount;
	uint32	FrameBindGeometryCount;
	uint32	FrameBindTextureCount;

	uint32 Framerate;
	uint32 CpuFrame;
	uint32 GpuFrame;
	uint32 CpuWait;
	uint32 DrawCalls;
	uint32 EffectBind;
	uint32 GeometryBind;
};

struct FeModuleRenderingInit : public FeModuleInit
{
public:
	uint32		Width;
	uint32		Height;
	bool		Debug;
	void*		WindowHandle;
	void*		WindowsInstance;
	int			WindowsCmdShow;
};

/// <summary>
/// Module main class to manage rendering
/// </summary>
class FeModuleRendering : public FeModule
{
public:
	static FeRenderDevice& GetDevice() { return Device; }

	virtual uint32 Load(const FeModuleInit*) override;
	virtual uint32 Unload() override;
	virtual uint32 Update(const FeDt& fDt) override;
	
	void SwitchDebugRenderTextMode();
	FeRenderBatch* CreateRenderBatch();
	void RegisterRenderBatch(FeRenderBatch* batch);
	void UnloadEffects();
	uint32 ReloadEffects();
	uint32 LoadEffects(const FeTArray<FeRenderEffect>& effects);
	const FeRenderViewport& GetDefaultViewport() const { return DefaultViewport; }

private:
	void BeginRender();
	void EndRender();
	void RenderBatch(const FeRenderBatch* batch, const FeDt& fDt);
	void RenderDebugText(const FeDt& fDt);

	static FeRenderDevice Device;
	std::map<FeResourceId, FeRenderEffect> Effects;
	FeTArray<FeRenderGeometryData> Geometries;
		
	FeTArray<FeRenderBatch*>		RegisteredRenderBatches;

	IFW1Factory*					FW1Factory;
	IFW1FontWrapper*				FontWrapper;

	char							DebugString[DEBUG_STRING_SIZE];
	FeEDebugRenderTextMode::Type	CurrentDebugTextMode;
	FeRenderDebugInfos				RenderDebugInfos;
	FeRenderViewport				DefaultViewport;
	FeModuleRenderingInit			RenderInit;
};
