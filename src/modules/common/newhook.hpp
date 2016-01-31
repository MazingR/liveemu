#ifndef DEFAULT_HEAP
#define DEFAULT_HEAP (unsigned int)-1
#endif
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

	void *FeMallocHook(size_t size, int iHeapId);
	void *FeCallocHook(size_t nmemb, size_t size, int iHeapId);
	void *FeReallocHook(void *ptr, size_t size, int iHeapIdiCopy, int iCopy);
	void FeFreeHook(void *ptr, int iHeapId);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <new>

#undef delete
#undef new

inline void operator delete (void* pAddress) _THROW0()
{
	FeFreeHook(pAddress, DEFAULT_HEAP);
}

inline void  operator delete  (void* pAddress, const std::nothrow_t&) _THROW0()
{
	FeFreeHook(pAddress, DEFAULT_HEAP);
}

inline void  operator delete[](void* pAddress) _THROW0()
{
	FeFreeHook(pAddress, DEFAULT_HEAP);
}

inline void  operator delete[](void* pAddress, const std::nothrow_t&) _THROW0()
{
	FeFreeHook(pAddress, DEFAULT_HEAP);
}

inline void* operator new   (std::size_t size) _THROW0()
{
	return FeMallocHook(size, DEFAULT_HEAP);
}
inline void* operator new[](std::size_t size) _THROW0()
{
	return FeMallocHook(size, DEFAULT_HEAP);
}
inline void* operator new   (std::size_t size, const std::nothrow_t&) _THROW0()
{
	return FeMallocHook(size, DEFAULT_HEAP);
}
inline void* operator new[](std::size_t size, const std::nothrow_t&) _THROW0()
{
	return FeMallocHook(size, DEFAULT_HEAP);
}
#endif