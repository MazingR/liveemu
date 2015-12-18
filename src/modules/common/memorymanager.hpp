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

	struct MemHeapDebugInfos
	{
		char	Name[COMMON_STR_SIZE];
		size_t	Allocated;
		size_t	AllocatedPeak;
		size_t	Size;
	};
	struct MemHeap
	{
		void*				HeapHandle;
		size_t				Size;
		MapAllocations		Allocations;
		MemHeapDebugInfos	DebugInfos;
	};

	class FeMemoryManager
	{
	public:
		FeMemoryManager();
		~FeMemoryManager();
		static FeMemoryManager StaticInstance;
		
		void*		Allocate(const size_t& _size, const size_t& _alignmemnt, int iHeapId);
		void*		Free(void* _ptr, int iHeapId);

		uint32		CreateHeapMBytes(const size_t& _size, const char* szName = "");
		uint32		CreateHeap(const size_t& _size, const char* szName = "");
		
		MemHeap&	GetHeap(int iHeapId);
		void		GetDebugInfos(char* outputStr, size_t outputStrSize);
	private:
		FeTArray<MemHeap>	Heaps;
	};
}
