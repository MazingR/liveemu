#include <pch.hpp>

#include <memorymanager.hpp>
#include <windows.h>

#define FE_LOCALASSERT(condition, fmt, ...) FE_ASSERT(condition, "[MemoryManager] "fmt, __VA_ARGS__) 
#define FE_LOCALLOG(fmt, ...) FE_LOG("[MemoryManager] "fmt, __VA_ARGS__) 

#define MEMALIGNEMENT 16
#define HOOK_MALLOC 1

HANDLE g_initHeapHandle = 0;
#define DEFAULT_HEAP_SIZE 32
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
	memset(outPtr, 0, size);
	return outPtr;
#else
	return calloc(nmemb, size);
#endif
}
void *FeReallocHook(void *ptr, size_t size, int iHeapId)
{
#if HOOK_MALLOC
	if (ptr)
		FeCommon::FeMemoryManager::StaticInstance.Free(ptr, iHeapId);

	return FeCommon::FeMemoryManager::StaticInstance.Allocate(size, MEMALIGNEMENT, iHeapId);
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
	: HeapsCount(0)
{
	size_t iHeapSize = DEFAULT_HEAP_SIZE*(1024 * 1024);

	g_initHeapHandle = HeapCreate(0, iHeapSize, iHeapSize);
}
MemHeap::MemHeap()
	: LocalBaseAdress(0)
	, LocalTotalSize(0)
	, TotalAllocatedSize(0)
	, PeakAllocatedSize(0)
{
	
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
	FE_ASSERT(HeapsCount < MAX_HEAP_COUNT, "Too many heaps created !");

	MemHeap& newHeap = Heaps[HeapsCount];
	
	newHeap.HeapHandle = HeapCreate(0, _size, 0);
	
	if (!newHeap.HeapHandle)
		return EFeReturnCode::Memory_CreateHeapFailed;

	newHeap.LocalBaseAdress = (uint64)HeapAlloc(newHeap.HeapHandle, 0, _size);
	if (!newHeap.LocalBaseAdress)
		return EFeReturnCode::Memory_CreateHeapFailed;

	newHeap.LocalTotalSize = _size;

	//	Add first memory chunk
	MemHeapFreeChunk& kChunk = newHeap.FreeChunks.Add();;
	// todo check adresses !

	kChunk.address	= newHeap.LocalBaseAdress;
	kChunk.chunkSize= _size;

	HeapsCount++;

	return EFeReturnCode::Success;
}

void* FeMemoryManager::Allocate(const uint64& _size, const uint64& _alignmemnt, int iHeapId)
{
	if (iHeapId == DEFAULT_HEAP)
	{
		void* outputPtr = HeapAlloc(g_initHeapHandle, 0, _size);
		FE_LOCALASSERT(outputPtr, "Not enough local memory on default heap ! (%d MB)", DEFAULT_HEAP_SIZE);
		return outputPtr;
	}

	MemHeap& heap = GetHeap(iHeapId);

	uint64 size = _size;
	MemHeapChunk	kAllocated;

	//	Try to find a free chunk
	uint64 smallest_chunk = (uint64)-1;
	uint64 freeChunkAlignOverhead = 0;
	
	uint32 iFreeChunksCount = heap.FreeChunks.GetSize();
	uint32 iFoundChunkIndex = (uint32)-1;
	
	for (uint32 i = 0; i < iFreeChunksCount; ++i)
	{
		const MemHeapFreeChunk& foundChunk = heap.FreeChunks[i];

		uint64 alignOverhead = ((foundChunk.address + _alignmemnt - 1) & (~(_alignmemnt - 1))) - foundChunk.address;
		
		//	find the smallest chunk
		if (foundChunk.chunkSize >= size + alignOverhead)
		{
			if (foundChunk.chunkSize < smallest_chunk)
			{
				smallest_chunk = foundChunk.chunkSize;
				iFoundChunkIndex = i;
				freeChunkAlignOverhead = alignOverhead;
			}
		}
	}
	FE_LOCALASSERT(iFoundChunkIndex != (uint32)-1, "Not enough local memory");

	//	Split the chunk
	MemHeapFreeChunk newFreeChunk = heap.FreeChunks[iFoundChunkIndex];
	heap.FreeChunks.RemoveAt(iFoundChunkIndex);

	uint64 left_memory;

	uint64 allocated_size = size + freeChunkAlignOverhead;

	heap.TotalAllocatedSize += allocated_size;
	if (heap.TotalAllocatedSize>heap.PeakAllocatedSize)
	{
		heap.PeakAllocatedSize = heap.TotalAllocatedSize;
	}

	kAllocated.address	= newFreeChunk.address;
	left_memory			= newFreeChunk.chunkSize - allocated_size;

	kAllocated.chunkSize= allocated_size;
	kAllocated.alignAddress	= (kAllocated.address + _alignmemnt-1) & (~(_alignmemnt-1));

#if CHECK_OVEWRITES
	for (uint32 index = 0; index < heap.AllocatedChunks.GetSize(); index++)
	{
		const MemHeapChunk&	chunk = heap.AllocatedChunks.GetAt(index);

		if(!((chunk.address < kAllocated.address && chunk.address + chunk.chunkSize <= kAllocated.address) ||
				(chunk.address >= kAllocated.address + kAllocated.chunkSize && chunk.address + chunk.chunkSize > kAllocated.address + kAllocated.chunkSize)))
		{
			FE_LOCALASSERT(false, "Overwrite");
		}
	}
#endif

	//	Register the allocated chunk
	heap.AllocatedChunks.Add(kAllocated);

	if(left_memory > 0)
	{
		//	Add a new free chunk
		newFreeChunk.address = kAllocated.address + allocated_size;		//	move MemHeap pointer
		newFreeChunk.chunkSize = left_memory;
		heap.FreeChunks.Add(newFreeChunk);
		// todo check adresses !
	}

	FE_LOCALASSERT((kAllocated.alignAddress & _alignmemnt-1) == 0, "bad alignement");

	return (void*)kAllocated.alignAddress;
}

void* FeMemoryManager::Free(void* _ptr, int iHeapId)
{
	if (iHeapId == DEFAULT_HEAP)
	{
		HeapFree(g_initHeapHandle, 0, _ptr);
		return NULL;
	}

	MemHeap& heap = GetHeap(iHeapId);

	uint64 addr = (uint64)_ptr;

	if(!_ptr)
	{
		FE_LOCALLOG("resource already freed");
		return NULL;
	}

	for (uint32 index = 0; index < heap.AllocatedChunks.GetSize(); ++index)
	{
		if (heap.AllocatedChunks[index].alignAddress == addr)
		{
			//	Return the found chunk to gobal pool
			const MemHeapChunk&	kAllocChunk = heap.AllocatedChunks[index];

			heap.TotalAllocatedSize -= kAllocChunk.chunkSize;
			
			MemHeapFreeChunk kFreeChunk;
			kFreeChunk.address = kAllocChunk.address;
			kFreeChunk.chunkSize = kAllocChunk.chunkSize;
			
			heap.AllocatedChunks.RemoveAtNoOrdering(index);

			MemHeapFreeChunk& newFreeChunk = heap.FreeChunks.Add();
			newFreeChunk.address = kAllocChunk.address;
			newFreeChunk.chunkSize = kAllocChunk.chunkSize;
			
			// todo check adresses

			return NULL;
		}
	}

	FE_LOCALASSERT(false, "Allocation not found !?");
	
	return NULL;
}
void FeMemoryManager::Defrag()
{
	for (uint32 i = 0; i < HeapsCount; ++i)
	{
		Heaps[i].Defrag();
	}
}

MemHeap& FeMemoryManager::GetHeap(int iHeapId)
{
	return Heaps[iHeapId];
}
void MemHeap::Defrag()
{
	FeTArray<MemHeapFreeChunk> oldFreeChunks(FreeChunks);
	FreeChunks.Clear();

	uint32 iFreeChunksCount = oldFreeChunks.GetSize();
	
	if (iFreeChunksCount < 2)
		return;

	// at least 1 free chunk
	uint64 uiStartAddress = oldFreeChunks[0].address;
	uint64 uiChunkSize = oldFreeChunks[0].chunkSize;

	for (uint32 i = 1; i < iFreeChunksCount; ++i)
	{
		const MemHeapFreeChunk& chunk = oldFreeChunks[i];

		if (chunk.address == (uiStartAddress + uiChunkSize))
		{
			// merge possible
			uiChunkSize += chunk.chunkSize;
		}
		else
		{
			// merge not possible
			MemHeapFreeChunk& newFreeChunk = FreeChunks.Add();
			newFreeChunk.address = uiStartAddress;
			newFreeChunk.chunkSize = uiChunkSize;

			uiStartAddress = chunk.address;
			uiChunkSize = chunk.chunkSize;
		}
	}
	
	MemHeapFreeChunk& newFreeChunk = FreeChunks.Add();
	newFreeChunk.address = uiStartAddress;
	newFreeChunk.chunkSize = uiChunkSize;
}

uint64 MemHeap::GetTotalFree() const
{
	uint64	size = 0;

	//MapMemFreeChunkConstIt it = MapMemFreeChunks.begin();
	//MapMemFreeChunkConstIt endIt = MapMemFreeChunks.end();
	//while (it != endIt)
	//{
	//	size += it->second;
	//	++it;
	//}
	return size;
}
}