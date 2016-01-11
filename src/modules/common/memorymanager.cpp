#include <memorymanager.hpp>
#include <windows.h>


#define FE_LOCALASSERT(condition, fmt, ...) FE_ASSERT(condition, "[MemoryManager] "fmt, __VA_ARGS__) 
#define FE_LOCALLOG(fmt, ...) FE_LOG("[MemoryManager] "fmt, __VA_ARGS__) 

#define MEMALIGNEMENT 16
#define DEFAULT_HEAP_SIZE 16
#define MEM_PAGE_SIZE_KB 512

bool g_IsMemoryManagerInit = false;

HANDLE GetDefaultHeapHandle()
{
	static HANDLE heapHandle = 0;

	if (!heapHandle)
	{
		size_t iHeapSize = DEFAULT_HEAP_SIZE*(1024 * 1024);
		heapHandle = HeapCreate(0, iHeapSize, iHeapSize);
	}

	return heapHandle;
}

FeMemoryManager FeMemoryManager::StaticInstance;

void* FeNewAllocate(size_t size, THeapId iHeapId)
{
	return FeMemoryManager::StaticInstance.Allocate(size, 16, iHeapId);
}
void* FeNewFree(void* ptr, THeapId iHeapId)
{
	return FeMemoryManager::StaticInstance.Free(ptr, iHeapId);
}

C_BEGIN
void *FeMallocHook(size_t size, int iHeapId)
{
#if HOOK_MALLOC
	if (g_IsMemoryManagerInit)
		return FeMemoryManager::StaticInstance.Allocate(size, MEMALIGNEMENT, iHeapId);
	else
		return HeapAlloc(GetDefaultHeapHandle(), 0, size);
#else
	return malloc(size);
#endif
}
void *FeCallocHook(size_t nmemb, size_t size, int iHeapId)
{
#if HOOK_MALLOC
	if (g_IsMemoryManagerInit)
	{
		void * outPtr = FeMemoryManager::StaticInstance.Allocate(nmemb*size, MEMALIGNEMENT, iHeapId);
		memset(outPtr, 0, nmemb*size);
		return outPtr;
	}
	else
	{
		return HeapAlloc(GetDefaultHeapHandle(), 0, nmemb*size);
	}
#else
	return calloc(nmemb, size);
#endif
}
void *FeReallocHook(void *ptr, size_t size, int iHeapId)
{
#if HOOK_MALLOC
	if (g_IsMemoryManagerInit)
	{
		void* output = FeMemoryManager::StaticInstance.Allocate(size, MEMALIGNEMENT, iHeapId);

		if (ptr)
		{
			memcpy(output, ptr, size);
			FeMemoryManager::StaticInstance.Free(ptr, iHeapId);
		}
		return output;
	}
	else
	{
		void* output = HeapAlloc(GetDefaultHeapHandle(), 0, size);

		if (ptr)
		{
			memcpy(output, ptr, size);
			HeapFree(GetDefaultHeapHandle(), 0, ptr);
	}
		return output;
	}
#else
	return realloc(ptr,size);
#endif
}
void FeFreeHook(void *ptr, int iHeapId)
{
#if HOOK_MALLOC
	if (g_IsMemoryManagerInit)
	{
		FeMemoryManager::StaticInstance.Free(ptr, iHeapId);
	}
	else
	{
		HeapFree(GetDefaultHeapHandle(), 0, ptr);
	}
#else
	free(ptr);
#endif
}
C_END

FeMemoryManager::FeMemoryManager()
{
	FE_ASSERT(false == g_IsMemoryManagerInit, "MemoryManager already init !");
	g_IsMemoryManagerInit = true;
}
FeMemoryManager::~FeMemoryManager()
{
}

uint32 FeMemoryManager::CreateHeapMBytes(const size_t& _size, const char* szName)
{
	return CreateHeap(_size* (1000 * 1000), szName);
}
uint32 FeMemoryManager::CreateHeap(const size_t& _size, const char* szName)
{
	MemHeap& newHeap = Heaps.Add();
	memset(&newHeap.DebugInfos, 0, sizeof(MemHeapDebugInfos));
	
	newHeap.HeapHandle = HeapCreate(0, _size, 0/*_size*/);

	if (!newHeap.HeapHandle)
	{
		FE_LOCALASSERT(newHeap.HeapHandle, "Heap creation failed !")
		return FeEReturnCode::Memory_CreateHeapFailed;
	}

	sprintf_s(newHeap.DebugInfos.Name, "%s", szName);
	newHeap.DebugInfos.Size = _size;
	newHeap.Size = _size;

	return FeEReturnCode::Success;
}

void* FeMemoryManager::Allocate(const size_t& _size, const size_t& _alignmemnt, int iHeapId)
{
	//FE_LOCALASSERT(MEM_PAGE_SIZE_KB>(_size / 1024), "can't allocate more than %d (KB) contiguous memory !", MEM_PAGE_SIZE_KB)

	if (iHeapId == DEFAULT_HEAP)
	{
		void* outputPtr = HeapAlloc(GetDefaultHeapHandle(), 0, _size);
		FE_LOCALASSERT(outputPtr, "Not enough local memory on default heap ! (%d MB)", DEFAULT_HEAP_SIZE);
		return outputPtr;
	}

	MemHeap& heap = GetHeap(iHeapId);

	void* ptr = HeapAlloc(heap.HeapHandle, 0, _size);
	FE_LOCALASSERT(ptr, "Not enought memory !");
	OnAllocate(heap, ptr, _size);

	return ptr;
}

void* FeMemoryManager::Free(void* _ptr, int iHeapId)
{
	if (!_ptr)
		return NULL;

	if (iHeapId == DEFAULT_HEAP)
	{
		HeapFree(GetDefaultHeapHandle(), 0, _ptr);
		return NULL;
	}

	MemHeap& heap = GetHeap(iHeapId);
	HeapFree(heap.HeapHandle, 0, _ptr);
	OnFree(heap, _ptr);

	return NULL;
}
MemHeap& FeMemoryManager::GetHeap(uint32 iHeapId)
{
	FE_LOCALASSERT(iHeapId < Heaps.GetSize(), "Invalid heap id !");
	return Heaps[iHeapId];
}
void FeMemoryManager::GetDebugInfos(char* outputStr, size_t outputStrSize)
{
	char szTmpStr[128];

	char* outputPtr = outputStr;
	size_t iOutputSize = outputStrSize;
	float fTotalAllocated = 0.f;
	float fTotalSize = 0.f;
	size_t iLineSize = 0;

	sprintf_s(szTmpStr, "Default Heap\t??%%    ?? / %d (MB)\n", DEFAULT_HEAP_SIZE);
	iLineSize = strlen(szTmpStr);
	sprintf_s(outputPtr, iOutputSize, "%s", szTmpStr);
	iOutputSize -= iLineSize;
	outputPtr += iLineSize;

	for (uint32 i = 0; i < Heaps.GetSize(); ++i)
	{
		MemHeapDebugInfos& infos = Heaps[i].DebugInfos;
		size_t iAllocated = infos.Allocated/1000;
		size_t iSize = infos.Size/ 1000;

		float fAllocatedMB = ((float)iAllocated) / 1024.0f;
		float fSizeMB = ((float)iSize) / 1024.0f;
		float fAllPercentMB = (fAllocatedMB / fSizeMB) * 100.0f;
		
		fTotalAllocated += fAllocatedMB;
		fTotalSize += fSizeMB;

		sprintf_s(szTmpStr, "Heap %s\t%4.2f%%    %4.2f / %4.0f (MB)\n", infos.Name, fAllPercentMB, fAllocatedMB, fSizeMB);

		sprintf_s(outputPtr, iOutputSize, "%s", szTmpStr);
		iLineSize = strlen(szTmpStr);

		iOutputSize -= iLineSize;
		outputPtr += iLineSize;
	}
	sprintf_s(outputPtr, iOutputSize, "Total \t%4.2f%%    %4.2f / %4.0f (MB)\n", (fTotalAllocated / fTotalSize) * 100.0f, fTotalAllocated, fTotalSize + DEFAULT_HEAP_SIZE);	
}
void FeMemoryManager::OnAllocate(MemHeap& heap, void* _ptr, const size_t& _size)
{
	heap.DebugInfos.Allocated += _size;
	if (heap.DebugInfos.Allocated > heap.DebugInfos.AllocatedPeak)
		heap.DebugInfos.AllocatedPeak = heap.DebugInfos.Allocated;

	std::pair<MapAllocationsIt, bool> bInsertResult;
	bInsertResult = heap.Allocations.insert(std::pair<size_t, size_t>((size_t)_ptr, _size));
	FE_LOCALASSERT(bInsertResult.second == true, "Insert allocation failed !");

}
void FeMemoryManager::OnFree(MemHeap& heap, void* _ptr)
{
	MapAllocationsIt it = heap.Allocations.find((size_t)_ptr);
	FE_LOCALASSERT(it != heap.Allocations.end(), "Allocation not found !?");
	heap.DebugInfos.Allocated -= it->second;
	heap.Allocations.erase(it);
}