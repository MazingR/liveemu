#pragma once

#include <commonrenderer.hpp>
#include <common/application.hpp>

uint32 ComputeResourceSizeInMemoryFromFormat(uint32 iWidth, uint32 iHeight, uint32 iResourceFormat, bool bHasAlpha);

// forward declares
struct SDL_mutex;
struct SDL_Thread;
struct  FT_LibraryRec_;

struct FeModuleRenderResourcesHandlerDebugInfos
{
	uint32 LoadedResourcesCount;
	uint32 ResourcesPoolSize;
	uint32 LoadedResourcesCountSizeInMemory;
};
class FeModuleRenderResourcesHandler : public FeModule
{

	typedef std::map<FeResourceId, FeRenderLoadingResource> ResourcesLoadingMap;
	typedef ResourcesLoadingMap::iterator ResourcesLoadingMapIt;
	typedef std::map<FeResourceId, FeRenderResource*> ResourcesMap;
	typedef ResourcesMap::iterator ResourcesMapIt;

	struct LockedLoadingResourcesMap
	{
		SDL_mutex*			Mutex;
		ResourcesLoadingMap	Resources;
	};
	

public:
	virtual uint32 Load(const FeModuleInit*) override;
	virtual uint32 Unload() override;
	virtual uint32 Update(const FeDt& fDt) override;

	uint32 LoadResource(FeRenderLoadingResource& resourceLoading);
	uint32 UnloadResource(const FeResourceId&);
	const FeRenderResource* GetResource(const FeResourceId&) const;

	void ComputeDebugInfos(FeModuleRenderResourcesHandlerDebugInfos& infos);
	uint32 ProcessThreadedResourcesLoading(bool& bThreadSopped);
	void UnloadResources();
private:
	uint32 LoadTexture(FeRenderLoadingResource& resource, FeRenderTexture* pTextureData);
	uint32 LoadFont(FeRenderLoadingResource& resource, FeRenderFont* pTextureData);
	uint32 PostLoadFont(FeRenderLoadingResource& resource, FeRenderFont* pTextureData);
	uint32 SaveTexture(FeRenderLoadingResource& resource, FeRenderTexture* pTextureData);

	std::map<FeEResourceLoadingState::Type, LockedLoadingResourcesMap> LoadingResources;
	SDL_Thread*			LoadingThread;
	SDL_mutex*			LoadingThreadMutex;
	ResourcesMap		Resources;

	size_t				ResourcePoolAllocated;
	size_t				ResourcePoolLimit;

	FT_LibraryRec_*		FtLibrary;
};