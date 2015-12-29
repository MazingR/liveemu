#pragma once

#include <commonrenderer.hpp>
#include <common/application.hpp>

// forward declares
struct SDL_mutex;
struct SDL_Thread;

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

		const FeRenderTexture* GetTexture(const FeRenderTextureId&) const;
		
		uint32 LoadTexture(const char*, FeRenderTextureId*);
		uint32 UnloadTexture(const FeRenderTextureId&);

		void ComputeDebugInfos(FeModuleRenderResourcesHandlerDebugInfos& infos);
		uint32 ProcessThreadedTexturesLoading();
	private:
		static uint32 ComputeTextureSizeInMemoryFromFormat(uint32 iWidth, uint32 iHeight, uint32 iTextureFormat, bool bHasAlpha);
		typedef std::map<FeRenderTextureId, FeRenderTexture> TexturesMap;
		typedef TexturesMap::iterator TexturesMapIt;

		typedef std::map<FeRenderTextureId, FeRenderLoadingTexture> TexturesLoadingMap;
		typedef TexturesLoadingMap::iterator TexturesLoadingMapIt;

		SDL_Thread*			LoadingThread;
		TexturesMap			Textures;
		
		TexturesLoadingMap	TexturesLoading;
		SDL_mutex*			TexturesLoadingMutex;

		TexturesLoadingMap	TexturesLoaded;
		SDL_mutex*			TexturesLoadedMutex;

		size_t				TexturePoolAllocated;
		size_t				TexturePoolLimit;
	};