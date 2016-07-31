#pragma once

#define MAX_HEAP_COUNT 64
#define MAX_ALOCC_MAP_SIZE 24000
#define ALLOC_MAP_COUNT 32

#include <common.hpp>
#include <tarray.hpp>
#include <unordered_map>
#include <memory>

typedef std::unordered_map<size_t, size_t>	MapAllocations;
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
	MapAllocations		Allocations[ALLOC_MAP_COUNT];
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
		
	MemHeap&	GetHeap(uint32 iHeapId);
	void		GetDebugInfos(char* outputStr, size_t outputStrSize);

	void OnAllocate(MemHeap& heap, void* _ptr, const size_t& _size);
	void OnFree(MemHeap& heap, void* _ptr);
private:
	FeTArray<MemHeap>	Heaps;
};