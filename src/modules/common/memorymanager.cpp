#include <pch.hpp>
#include <memorymanager.hpp>
#include <windows.h>


#define FE_LOCALASSERT(condition, fmt, ...) FE_ASSERT(condition, "[MemoryManager] "fmt, __VA_ARGS__) 
#define FE_LOCALLOG(fmt, ...) FE_LOG("[MemoryManager] "fmt, __VA_ARGS__) 

#define MEMALIGNEMENT 16

HANDLE g_initHeapHandle = 0;
#define DEFAULT_HEAP_SIZE 16

#define MEM_PAGE_SIZE 512*(1000)
#define CHECK_OVEWRITES 1

namespace FeCommon
{
	FeMemoryManager FeMemoryManager::StaticInstance;
}

void* FeNewAllocate(size_t size, THeapId iHeapId)
{
	return FeCommon::FeMemoryManager::StaticInstance.Allocate(size, 16, iHeapId);
}
void* FeNewFree(void* ptr, THeapId iHeapId)
{
	return FeCommon::FeMemoryManager::StaticInstance.Free(ptr, iHeapId);
}

C_BEGIN
void *FeMallocHook(size_t size, int iHeapId)
{
#if HOOK_MALLOC
	return FeCommon::FeMemoryManager::StaticInstance.Allocate(size, MEMALIGNEMENT, iHeapId);
#else
	return malloc(size);
#endif
}
void *FeCallocHook(size_t nmemb, size_t size, int iHeapId)
{
#if HOOK_MALLOC
	void * outPtr = FeCommon::FeMemoryManager::StaticInstance.Allocate(nmemb*size, MEMALIGNEMENT, iHeapId);
	memset(outPtr, 0, nmemb*size);
	return outPtr;
#else
	return calloc(nmemb, size);
#endif
}
void *FeReallocHook(void *ptr, size_t size, int iHeapId)
{
#if HOOK_MALLOC
	void* output = FeCommon::FeMemoryManager::StaticInstance.Allocate(size, MEMALIGNEMENT, iHeapId);

	if (ptr)
	{
		memcpy(output, ptr, size);
		FeCommon::FeMemoryManager::StaticInstance.Free(ptr, iHeapId);
	}
	return output;
#else
	return realloc(ptr,size);
#endif
}
void FeFreeHook(void *ptr, int iHeapId)
{
#if HOOK_MALLOC
	FeCommon::FeMemoryManager::StaticInstance.Free(ptr, iHeapId);
#else
	free(ptr);
#endif
}
C_END

namespace FeCommon
{
FeMemoryManager::FeMemoryManager()
{
	size_t iHeapSize = DEFAULT_HEAP_SIZE*(1024 * 1024);
	g_initHeapHandle = HeapCreate(0, iHeapSize, iHeapSize);
}
FeMemoryManager::~FeMemoryManager()
{
}

uint32 FeMemoryManager::CreateHeapMBytes(const size_t& _size)
{
	return CreateHeap(_size* (1000 * 1000));
}
uint32 FeMemoryManager::CreateHeap(const size_t& _size)
{
	MemHeap& newHeap = Heaps.Add();
	
	newHeap.HeapHandle = HeapCreate(0, _size, _size);
	if (!newHeap.HeapHandle)
	{
		FE_LOCALASSERT(newHeap.HeapHandle, "Heap creation failed !")
		return EFeReturnCode::Memory_CreateHeapFailed;
	}

	newHeap.Size = _size;

	return EFeReturnCode::Success;
}

void* FeMemoryManager::Allocate(const size_t& _size, const size_t& _alignmemnt, int iHeapId)
{
	if (iHeapId == DEFAULT_HEAP)
	{
		void* outputPtr = HeapAlloc(g_initHeapHandle, 0, _size);
		FE_LOCALASSERT(outputPtr, "Not enough local memory on default heap ! (%d MB)", DEFAULT_HEAP_SIZE);
		return outputPtr;
	}

	MemHeap& heap = GetHeap(iHeapId);

	void* ptr = HeapAlloc(heap.HeapHandle, 0, _size);
	FE_LOCALASSERT(ptr, "Not enought memory !");

	std::pair<MapAllocationsIt, bool> bInsertResult;
	bInsertResult = heap.Allocations.insert(std::pair<size_t, size_t>((size_t)ptr, _size));
	FE_LOCALASSERT(bInsertResult.second == true, "Insert allocation failed !");

	return ptr;
}

void* FeMemoryManager::Free(void* _ptr, int iHeapId)
{
	if (iHeapId == DEFAULT_HEAP)
	{
		HeapFree(g_initHeapHandle, 0, _ptr);
		return NULL;
	}

	MemHeap& heap = GetHeap(iHeapId);

	HeapFree(heap.HeapHandle, 0, _ptr);
	MapAllocationsIt it = heap.Allocations.find((size_t)_ptr);
	FE_LOCALASSERT(it != heap.Allocations.end(), "Allocation not found !?");
	heap.Allocations.erase(it);
	
	return NULL;
}
MemHeap& FeMemoryManager::GetHeap(int iHeapId)
{
	FE_LOCALASSERT(iHeapId < Heaps.GetSize(), "Invalid heap id !");
	return Heaps[iHeapId];
}

}