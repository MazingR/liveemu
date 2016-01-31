#ifndef DEFAULT_HEAP
#define DEFAULT_HEAP (uint32)-1
#endif

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
	void* FeMallocHook(size_t size, int iHeapId);
	void* FeCallocHook(size_t nmemb, size_t size, int iHeapId);
	void* FeReallocHook(void *ptr, size_t size, int iHeapId, int iCopy);
	void FeFreeHook(void *ptr, int iHeapId);

#ifdef __cplusplus
	namespace std
	{
		inline void* std_FeMallocHook(size_t size, int iHeapId)							{ return FeMallocHook(size, iHeapId); }
		inline void* std_FeCallocHook(size_t nmemb, size_t size, int iHeapId)			{ return FeCallocHook(nmemb, size, iHeapId); }
		inline void* std_FeReallocHook(void *ptr, size_t size, int iHeapId, int iCopy)	{ return FeReallocHook(ptr, size, iHeapId, iCopy); }
		inline void std_FeFreeHook(void *ptr, int iHeapId)								{ FeFreeHook(ptr, iHeapId); }
	}
#endif
#ifdef __cplusplus
}
#endif
#define HOOK_MALLOC 1

#if HOOK_MALLOC
#include <malloc.h>

#define _real_malloc(size)			malloc(size)
#define _real_calloc(nmemb, size)	calloc(nmemb, size)
#define _real_realloc(ptr, size)	realloc(ptr, size,1)
#define _real_free(ptr)				free(ptr)

#undef malloc
#undef calloc
#undef realloc
#undef free

#define malloc(size)		FeMallocHook(size, DEFAULT_HEAP)
#define calloc(nmemb, size)	FeCallocHook(nmemb, size, DEFAULT_HEAP)
#define realloc(ptr, size)	FeReallocHook(ptr, size, DEFAULT_HEAP, 1)
#define free(ptr)			FeFreeHook(ptr, DEFAULT_HEAP)

#define FE_ALLOCATE(size, heap) FeMallocHook(size, heap)
#define FE_FREE(ptr, heap) FeFreeHook(ptr, heap)

#define FE_ALLOCATED(size) FeMallocHook(size, DEFAULT_HEAP)
#define FE_FREED(ptr) FeFreeHook(ptr, DEFAULT_HEAP)

#define _CRT_ALLOCATION_DEFINED // we don't want to bother with crt defines
#endif