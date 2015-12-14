#pragma once

#include <common/common.hpp>

#include <geometry.hpp>
#include <effect.hpp>
#include <device.hpp>

#pragma warning( disable : 4005)
#include <d3d11.h>

namespace FeRendering
{
	struct FeRenderViewport
	{
	public:
		uint32 CreateFromBackBuffer();
		ID3D11RenderTargetView* GetRenderTargetView() { return RenderTargetView; }

		void Bind()  const;
		void Clear()  const;
	private:
		ID3D11RenderTargetView* RenderTargetView;
	};

	struct FeRenderBatch
	{
		FeCommon::FeTArray<FeRenderGeometryInstance> GeometryInstances;
		FeRenderViewport Viewport;
	};

	/// <summary>
	/// Module main class to manage rendering
	/// </summary>
	class FeModuleRendering : public ::FeCommon::FeModule
	{
	public:

		virtual uint32 Load(const ::FeCommon::FeModuleInit*) override;
		virtual uint32 Unload() override;
		virtual uint32 Update() override;
		static FeRenderDevice& GetDevice() { return Device; }

	private:
		static FeRenderDevice Device;
		void RenderBatch(FeRenderBatch& batch);
		
		FeCommon::FeTArray<FeRenderEffect> Effects;
		FeCommon::FeTArray<FeRenderGeometryData> Geometries;

		FeRenderBatch renderBatch;

	};

	struct FeModuleRenderingInit : public ::FeCommon::FeModuleInit
	{
	public:
		uint32		Width;
		uint32		Height;
		bool		Debug;
		HWND		WindowHandle;
		HINSTANCE	WindowsInstance;
		int			WindowsCmdShow;
	};
}