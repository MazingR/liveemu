#ifndef DEFAULT_HEAP
#define DEFAULT_HEAP 0
#endif

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

	void *FeMallocHook(size_t size, int iHeapId);
	void *FeCallocHook(size_t nmemb, size_t size, int iHeapId);
	void *FeReallocHook(void *ptr, size_t size, int iHeapId);
	void FeFreeHook(void *ptr, int iHeapId);

#ifdef __cplusplus
}
#endif

#undef malloc
#undef calloc
#undef realloc
#undef free

#define malloc(size) FeMallocHook(size, DEFAULT_HEAP)
#define calloc(nmemb, size) FeCallocHook(nmemb, size, DEFAULT_HEAP)
#define realloc(ptr, size) FeReallocHook(ptr, size, DEFAULT_HEAP)
#define free(ptr) FeFreeHook(ptr, DEFAULT_HEAP)

#define _CRT_ALLOCATION_DEFINED // we don't want to bother with crt defines