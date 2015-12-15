#include "memorymanager.hpp"

#define FE_LOCALASSERT(condition, fmt, ...) FE_ASSERT(condition, "[MemoryManager] "fmt, __VA_ARGS__) 

namespace FeCommon
{
}

void *FeMallocHook(size_t size, int iHeapId)
{
}
void *FeCallocHook(size_t nmemb, size_t size, int iHeapId)
{
}
void *FeReallocHook(void *ptr, size_t size, int iHeapId)
{

}
void FeFreeHook(void *ptr, int iHeapId)
{
}

namespace FeCommon
{
FeMemoryManager::FeMemoryManager()
	: LocalBaseAdress(0)
	, TotalAllocatedSize(0)
	, PeakAllocatedSize(0)
{
}

FeMemoryManager::~FeMemoryManager()
{
}

void FeMemoryManager::GetLocalMemoryHeapAddress(uint64** _baseAddress)
{
	*_baseAddress = &LocalBaseAdress;
};

void FeMemoryManager::Init(const uint64& _baseAddress, const uint64& _localSize)
{
	memset(&ChunckAllocatedSize[0], 0, sizeof(uint64) * NUM_CHUNK_TYPE);
	memset(&AllocationStats[0], 0, sizeof(uint64) * NUM_CHUNK_TYPE);

	FE_LOCALASSERT(LocalBaseAdress == 0, "WARNING ! a local memory has been already set !");

	LocalBaseAdress	= _baseAddress;
	LocalTotalSize	= _localSize;

	//	Add first memory chunk
	FreeMemoryChunk		kChunk;

	kChunk.address	= _baseAddress;
	kChunk.chunkSize= _localSize;

	AllocationStats[FREE_CHUNK]++;

	std::pair<MapFreeChunkIt, bool> bInsertResult;
	bInsertResult = MapFreeChunks.insert(std::pair<uint64,uint64>(kChunk.address, kChunk.chunkSize));
	FE_LOCALASSERT(bInsertResult.second == true, "insertion of free memory chunk failed !!!");
}

void* FeMemoryManager::Allocate(const uint64& _size, const uint64& _alignmemnt, ChunkType _type)
{
	FE_LOCALASSERT(_type < NUM_CHUNK_TYPE, "Bad Video Memory Chunk Type");

	uint64 size = _size;
	MemoryChunk	kAllocated;

	//	Try to find a free chunk
	uint64 smallest_chunk = (uint64)-1;
	uint64 freeChunkAlignOverhead = 0;

	MapFreeChunkIt it = MapFreeChunks.begin();
	MapFreeChunkIt endIt = MapFreeChunks.end();
	MapFreeChunkIt foundIt = MapFreeChunks.end();

	while(it != endIt)
	{
		uint64 alignOverhead = ((it->first + _alignmemnt-1) & (~(_alignmemnt-1))) - it->first;
		//	find the smallest chunk
		if(it->second >= size + alignOverhead)
		{
			if(it->second < smallest_chunk)
			{
				smallest_chunk	= it->second;
				foundIt = it;
				freeChunkAlignOverhead = alignOverhead;
			}				
		}
		++it;
	}
	FE_LOCALASSERT(foundIt != endIt, "Not enough local memory");

	//	Split the chunk
	FreeMemoryChunk	freeChunk;
	freeChunk.address = foundIt->first;
	freeChunk.chunkSize = foundIt->second;
	uint64 left_memory;

	uint64 allocated_size = size + freeChunkAlignOverhead;

	TotalAllocatedSize += allocated_size;
	if(TotalAllocatedSize>PeakAllocatedSize)
	{
		PeakAllocatedSize=TotalAllocatedSize;
	}
	ChunckAllocatedSize[_type] += allocated_size;

	kAllocated.type		= _type;
	kAllocated.address	= freeChunk.address;
	left_memory			= freeChunk.chunkSize - allocated_size;

	AllocationStats[FREE_CHUNK]--;
	MapFreeChunks.erase(foundIt);

	kAllocated.chunkSize= allocated_size;
	kAllocated.alignAddress	= (kAllocated.address + _alignmemnt-1) & (~(_alignmemnt-1));

	for(uint64 index = 0; index < AllocatedChunks.GetSize(); index++)
	{
		MemoryChunk	chunk = AllocatedChunks.GetAt(index);

		if(!((chunk.address < kAllocated.address && chunk.address + chunk.chunkSize <= kAllocated.address) ||
				(chunk.address >= kAllocated.address + kAllocated.chunkSize && chunk.address + chunk.chunkSize > kAllocated.address + kAllocated.chunkSize)))
		{
			FE_LOCALASSERT(false, "Overwrite");
		}
	}

	//	Register the allocated chunk
	AllocatedChunks.Add(kAllocated);

	if(left_memory > 0)
	{
		//	Add a new free chunk
		freeChunk.address	=	kAllocated.address + allocated_size;		//	move heap pointer
		freeChunk.chunkSize	=	left_memory;
		AllocationStats[FREE_CHUNK]++;

		std::pair<MapFreeChunkIt, bool> bInsertResult;
		bInsertResult = MapFreeChunks.insert(std::pair<uint64, uint64>(freeChunk.address, freeChunk.chunkSize));
		FE_LOCALASSERT(bInsertResult.second == true, "Insertion of free memory chunk failed !!!");
	}

	AllocationStats[_type]++;

	FE_LOCALASSERT((kAllocated.alignAddress & _alignmemnt-1) == 0, "bad alignement");

	return (void*)kAllocated.alignAddress;
}

void* FeMemoryManager::Free(void* _ptr)
{
	uint64 addr = (uint64)_ptr;

	if(!_ptr)
	{
		FE_LOCALASSERT(false, "resource already freed");
		return NULL;
	}

	for(uint64 index = 0; index < AllocatedChunks.GetSize(); ++index)
	{
		if(AllocatedChunks[index].alignAddress == addr)
		{
			//	Return the found chunk to gobal pool
			const MemoryChunk&	kAllocChunk = AllocatedChunks[index];

			TotalAllocatedSize -= kAllocChunk.chunkSize;
			ChunckAllocatedSize[kAllocChunk.type] -= kAllocChunk.chunkSize;
				
			AllocationStats[kAllocChunk.type]--;
			AllocationStats[FREE_CHUNK]++;

			FreeMemoryChunk kFreeChunk;
			kFreeChunk.address = kAllocChunk.address;
			kFreeChunk.chunkSize = kAllocChunk.chunkSize;

			AllocatedChunks.RemoveAtNoOrdering(index);

			std::pair<MapFreeChunkIt, bool> bInsertResult;
			bInsertResult = MapFreeChunks.insert(std::pair<uint64, uint64>(kFreeChunk.address, kFreeChunk.chunkSize));
			FE_LOCALASSERT(bInsertResult.second == true, "Insertion of free memory chunk failed !!!");

			return NULL;
		}
	}

	FE_LOCALASSERT(false, "Allocation not found !?");
	
	return NULL;
}

void FeMemoryManager::Defrag()
{
	MapFreeChunk mapNewFreeChunk;

	MapFreeChunkIt it = MapFreeChunks.begin();
	MapFreeChunkIt endIt = MapFreeChunks.end();

	if(it != endIt)
	{
		// at least 1 free chunk
		uint64 uiStartAddress = it->first;
		uint64 uiChunkSize = it->second;
		++it;

		while(it != endIt)
		{
			if(it->first == (uiStartAddress + uiChunkSize))
			{
				// merge possible
				uiChunkSize += it->second;
			}
			else
			{
				// merge not possible
				mapNewFreeChunk.insert(std::pair<uint64, uint64>(uiStartAddress, uiChunkSize));
				uiStartAddress = it->first;
				uiChunkSize = it->second;
			}
			++it;
		}

		std::pair<MapFreeChunkIt, bool> bInsertResult;
		bInsertResult = mapNewFreeChunk.insert(std::pair<uint64, uint64>(uiStartAddress, uiChunkSize));
		FE_LOCALASSERT(bInsertResult.second == true, "Insertion of free memory defrag chunk failed !!!");
	}

	AllocationStats[FREE_CHUNK] = mapNewFreeChunk.size();
	MapFreeChunks.swap(mapNewFreeChunk);
}

uint64 FeMemoryManager::GetTotalFree() const
{
	uint64	size = 0;

	MapFreeChunkConstIt it = MapFreeChunks.begin();
	MapFreeChunkConstIt endIt = MapFreeChunks.end();
	while (it != endIt)
	{
		size += it->second;
		++it;
	}
	return size;
}
}