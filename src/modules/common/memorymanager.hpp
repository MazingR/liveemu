#pragma once

#define MAX_HEAP_COUNT 2

#include <common.hpp>
#include <tarray.hpp>
#include <unordered_map>
#include <memory>

namespace FeCommon
{
	typedef std::unordered_map<uint64, uint64>	MapMemFreeChunk;
	typedef MapMemFreeChunk::iterator			MapMemFreeChunkIt;
	typedef MapMemFreeChunk::const_iterator		MapMemFreeChunkConstIt;

	typedef struct MemHeapChunk
	{
		uint64		address;
		uint64		alignAddress;
		uint64		chunkSize;

		MemHeapChunk()
			: address(0)
			, alignAddress(0)
			, chunkSize(0)
		{

		}
	} MemHeapChunk;

	typedef struct MemHeapFreeChunk
	{
		uint64		address;
		uint64		chunkSize;
	} MemHeapFreeChunk;

	typedef struct MemHeap
	{
		void*						HeapHandle;
		FeTArray<MemHeapFreeChunk>	FreeChunks;
		FeTArray<MemHeapChunk>		AllocatedChunks;
		uint64						LocalBaseAdress;
		uint64						LocalTotalSize;
		uint64						TotalAllocatedSize;
		uint64						PeakAllocatedSize;

		uint64		GetTotalFree() const;
		void		Defrag();

		MemHeap();
	} MemHeap;

	class FeMemoryManager
	{
	public:
		FeMemoryManager();
		~FeMemoryManager();
		static FeMemoryManager StaticInstance;
		
		void*		Allocate(const uint64& _size, const uint64& _alignmemnt, int iHeapId);
		void*		Free(void* _ptr, int iHeapId);

		uint32		CreateHeapMBytes(const size_t& _size);
		uint32		CreateHeap(const size_t& _size);
		void		Defrag();
		
		MemHeap& GetHeap(int iHeapId);

	private:
		uint32		HeapsCount;
		MemHeap		Heaps[MAX_HEAP_COUNT];
	};
}
