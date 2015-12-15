#include <cstdlib>

#ifndef CURRENT_HEAP_ID
#define CURRENT_HEAP_ID 0
#endif

extern void *FeMallocHook(size_t size, int iHeapId);
extern void *FeCallocHook(size_t nmemb, size_t size, int iHeapId);
extern void *FeReallocHook(void *ptr, size_t size, int iHeapId);
extern void FeFreeHook(void *ptr, int iHeapId);

#undef malloc
#undef calloc
#undef realloc
#undef free

#define malloc(size) FeMallocHook(size, CURRENT_HEAP_ID)
#define calloc(nmemb, size) FeCallocHook(nmemb, size, CURRENT_HEAP_ID)
#define realloc(ptr, size) FeReallocHook(ptr, size, CURRENT_HEAP_ID)
#define free(ptr) FeFreeHook(ptr, CURRENT_HEAP_ID)