#pragma once

#define MAX_HEAP_COUNT 64

#include <common.hpp>
#include <tarray.hpp>
#include <unordered_map>
#include <memory>

namespace FeCommon
{
	typedef std::unordered_map<uint64, uint64>	MapAllocations;
	typedef MapAllocations::iterator			MapAllocationsIt;
	typedef MapAllocations::const_iterator		MapAllocationsConstIt;

	typedef struct MemHeap
	{
		void*			HeapHandle;
		size_t			Size;
		MapAllocations	Allocations;
	} MemHeap;

	class FeMemoryManager
	{
	public:
		FeMemoryManager();
		~FeMemoryManager();
		static FeMemoryManager StaticInstance;
		
		void*		Allocate(const size_t& _size, const size_t& _alignmemnt, int iHeapId);
		void*		Free(void* _ptr, int iHeapId);

		uint32		CreateHeapMBytes(const size_t& _size);
		uint32		CreateHeap(const size_t& _size);
		
		MemHeap& GetHeap(int iHeapId);

	private:
		FeTArray<MemHeap>	Heaps;
	};
}
