#pragma once

#include <common/common.hpp>
#include <common/application.hpp>

#include <geometry.hpp>
#include <effect.hpp>
#include <device.hpp>

#pragma warning( disable : 4005)
#include <d3d11.h>

struct SDL_mutex;

#define DEBUG_STRING_SIZE 1024
#define RENDERER_HEAP 2

// forward declares
interface IFW1Factory;
interface IFW1FontWrapper;

namespace FeRendering
{
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

	struct FeTexturePath
	{
		char Str[COMMON_PATH_SIZE];
	};
	struct FeRenderTexture
	{
		FeTexturePath					Path;
		FeETextureLoadingState::Type	LoadingState;
		ID3D11Resource*					Resource;
		ID3D11ShaderResourceView*		SRV;
	};

	struct FeTextureLoadingQueryResult
	{
		FeRenderTextureId TextureId;
	};

	class FeModuleRenderResourcesHandler : public ::FeCommon::FeModule
	{
	public:
		virtual uint32 Load(const ::FeCommon::FeModuleInit*) override;
		virtual uint32 Unload() override;
		virtual uint32 Update(const FeDt& fDt) override;

		bool IsLoaded(const FeRenderTextureId&);
		bool IsLoading(const FeRenderTextureId&);
		const FeRenderTexture* GetTexture(const FeRenderTextureId&) const;
		
		uint32 LoadTexture(const char*, FeRenderTextureId*);
		uint32 UnloadTexture(const FeRenderTextureId&);

	private:
		typedef std::map<FeRenderTextureId, FeRenderTexture> TexturesMap;

		TexturesMap		Textures;
		SDL_mutex*		TexturesMapMutex;
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

	struct FeRenderDebugInfos
	{
		uint32	FrameDrawCallsCount;
		uint32	FrameBindEffectCount;
		uint32	FrameBindGeometryCount;

		uint32 Framerate;
		uint32 CpuFrame;
		uint32 GpuFrame;
		uint32 CpuWait;
		uint32 DrawCalls;
		uint32 EffectBind;
		uint32 GeometryBind;
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
		virtual uint32 Update(const FeDt& fDt) override;

		static FeRenderDevice& GetDevice() { return Device; }
		void SwitchDebugRenderTextMode();
	private:
		void BeginRender();
		void EndRender();
		void RenderBatch(FeRenderBatch& batch, const FeDt& fDt);
		void RenderDebugText(const FeDt& fDt);

		static FeRenderDevice Device;
		FeCommon::FeTArray<FeRenderEffect> Effects;
		FeCommon::FeTArray<FeRenderGeometryData> Geometries;
		
		FeRenderBatch renderBatch;

		IFW1Factory*					FW1Factory;
		IFW1FontWrapper*				FontWrapper;
		char							DebugString[DEBUG_STRING_SIZE];
		FeEDebugRenderTextMode::Type	CurrentDebugTextMode;
		FeRenderDebugInfos				RenderDebugInfos;
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