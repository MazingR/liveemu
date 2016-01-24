#pragma once

#include <commonrenderer.hpp>
#include <common/application.hpp>

// forward declares
struct SDL_mutex;
struct SDL_Thread;

struct FeModuleRenderResourcesHandlerDebugInfos
{
	uint32 LoadedResourcesCount;
	uint32 ResourcesPoolSize;
	uint32 LoadedResourcesCountSizeInMemory;
};

class FeModuleRenderResourcesHandler : public FeModule
{
public:
	virtual uint32 Load(const FeModuleInit*) override;
	virtual uint32 Unload() override;
	virtual uint32 Update(const FeDt& fDt) override;

	uint32 LoadResource(FeRenderLoadingResource& resourceLoading);
	uint32 UnloadResource(const FeResourceId&);
	const FeRenderResource* GetResource(const FeResourceId&) const;

	void ComputeDebugInfos(FeModuleRenderResourcesHandlerDebugInfos& infos);
	uint32 ProcessThreadedResourcesLoading(bool& bThreadSopped);
private:
	static uint32 ComputeResourceSizeInMemoryFromFormat(uint32 iWidth, uint32 iHeight, uint32 iResourceFormat, bool bHasAlpha);

	typedef std::map<FeResourceId, FeRenderResource> ResourcesMap;
	typedef ResourcesMap::iterator ResourcesMapIt;

	typedef std::map<FeResourceId, FeRenderLoadingResource> ResourcesLoadingMap;
	typedef ResourcesLoadingMap::iterator ResourcesLoadingMapIt;

	uint32 CreateTexture(FeRenderLoadingResource& resource, FeRenderTextureData* pTextureData);
	uint32 SaveTexture(FeRenderLoadingResource& resource, FeRenderTextureData* pTextureData);


	SDL_Thread*			LoadingThread;
		
	ResourcesMap		Resources;
	
	ResourcesLoadingMap	ResourcesToSave;
	ResourcesLoadingMap	ResourcesLoading;

	SDL_mutex*			ResourcesLoadingMutex;

	ResourcesLoadingMap	ResourcesLoaded;
	SDL_mutex*			ResourcesLoadedMutex;

	size_t				ResourcePoolAllocated;
	size_t				ResourcePoolLimit;
};