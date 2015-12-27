#pragma once

#include <commonrenderer.hpp>
#include <common/application.hpp>

// forward declares
struct SDL_mutex;

	struct FeModuleRenderResourcesHandlerDebugInfos
	{
		uint32 LoadedTexturesCount;
		uint32 LoadedTexturesCountSizeInMemory;
	};
	class FeModuleRenderResourcesHandler : public FeModule
	{
	public:
		virtual uint32 Load(const FeModuleInit*) override;
		virtual uint32 Unload() override;
		virtual uint32 Update(const FeDt& fDt) override;

		bool IsLoaded(const FeRenderTextureId&);
		bool IsLoading(const FeRenderTextureId&);
		const FeRenderTexture* GetTexture(const FeRenderTextureId&) const;
		
		uint32 LoadTexture(const char*, FeRenderTextureId*);
		uint32 UnloadTexture(const FeRenderTextureId&);

		void ComputeDebugInfos(FeModuleRenderResourcesHandlerDebugInfos& infos);
	private:
		static uint32 ComputeTextureSizeInMemoryFromFormat(uint32 iWidth, uint32 iHeight, uint32 iTextureFormat, bool bHasAlpha);
		typedef std::map<FeRenderTextureId, FeRenderTexture> TexturesMap;
		typedef TexturesMap::iterator TexturesMapIt;

		TexturesMap		Textures;
		SDL_mutex*		TexturesMapMutex;
	};