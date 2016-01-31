#pragma once 

#include <stdio.h>

#ifdef __cplusplus
#define C_BEGIN extern "C" {
#else
#define C_BEGIN
#endif

#ifdef __cplusplus
#define C_END }
#else
#define C_END
#endif

#define SafeRelease(a) {if (a) a->Release();}
#define STR_SIZE_SMALL 32
#define STR_SIZE_MEDIUM 128
#define STR_SIZE_WIDE 512

#define COMMON_STR_SIZE 128
#define COMMON_PATH_SIZE 512

#ifdef DEBUG
#define CONFIGSTR "Debug"
#elif defined(MASTER)
#define CONFIGSTR "Master"
#else
#define CONFIGSTR "Release"
#endif

#include <newhook.hpp>
#include <mallochook.hpp>

typedef unsigned int THeapId;
#define INIT_HEAP_ID ((THeapId)-1)

typedef unsigned char uint8;
typedef short unsigned int uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef short int int16;
typedef int int32;
typedef long long int64;

namespace FeEReturnCode
{
	enum Type
	{
		Success = 0,
		Failed,
		Canceled,

		File_OpenFailed,
		File_ReadFailed,

		Rendering_CreateShaderFailed,

		Memory_CreateHeapFailed,
		Memory_AllocationFailedNotEnoughMemory,

		FileSystem_Error,
	};
};

int __cdecl vs_printf(const char *format, ...);

void FeSetLastError(const char* fmt, ...);
const char* FeGetLastError();

#define FE_FAILED(a) (a!=FeEReturnCode::Success)
#define FE_FAILEDRETURN(a) { uint32 ___iResult = (a); { if FE_FAILED(___iResult) return ___iResult; } }

#define FE_LOG(fmt, ...)				\
	vs_printf(fmt, __VA_ARGS__);		\
	vs_printf("\n");					\
	FeSetLastError(fmt, __VA_ARGS__);	\
	

#ifdef DEBUG
	#define FE_ASSERT(condition, fmt, ...) { if (!(condition)) { FE_LOG(fmt, __VA_ARGS__);__debugbreak();  } }
#else
	#define FE_ASSERT(condition, fmt, ...) { if (!(condition)) { FE_LOG(fmt, __VA_ARGS__); } }
#endif

#define FE_NEW(type, heap) FeNew<type>(heap)
#define FE_NEW_ARRAY(type, size, heap) FeNewA<type>(size, heap)

#define FE_NEWD(type) FeNewA<type>(size, DEFAULT_HEAP)
#define FE_NEW_ARRAYD(type, size) FeNewA<type>(size, DEFAULT_HEAP)

#define FE_DELETE(type, ptr, heap) FeDelete<type>(ptr, heap)
#define FE_DELETE_ARRAY(type, ptr, size, heap) FeDeleteA<type>(ptr, size, heap)

#define FE_DELETED(type, ptr) FeDelete<type>(ptr, DEFAULT_HEAP)
#define FE_DELETE_ARRAYD(type, ptr, size) FeDeleteA<type>(ptr, size, DEFAULT_HEAP)

typedef struct FeRect
{
	int x, y;
	int w, h;
} FeRect;

void* FeNewAllocate(size_t size, THeapId iHeapId);
void* FeNewFree(void* ptr, THeapId iHeapId);

template<typename T>
inline T* FeNew(THeapId iHeapId = DEFAULT_HEAP)
{
	void* ptr = FeNewAllocate(sizeof(T), iHeapId);
	new (ptr) T;

	return (T*)ptr;
}
template<typename T>
inline T* FeNewA(size_t count, THeapId iHeapId = DEFAULT_HEAP)
{
	void* ptr = FeNewAllocate(sizeof(T)*count, iHeapId);

	for (size_t i = 0; i < count; ++i)
		new (((T*)(ptr)) + i)T;
	return (T*)ptr;
}
template<typename T>
void FeDelete(T* ptr, THeapId iHeapId = DEFAULT_HEAP)
{
	((T*)(ptr))->~T();
	FeNewFree(ptr, iHeapId);
}
template<typename T>
void FeDeleteA(void* ptr, size_t count, THeapId iHeapId = DEFAULT_HEAP)
{
	for (size_t i = 0; i < count; ++i)
		(((T*)(ptr))+i)->~T();

	FeNewFree(ptr, iHeapId);
}

struct FeDt
{
	float	TotalSeconds;
	uint32	TotalMilliseconds;
	int		TotalCpuWaited;
};

struct FePath
{
	char Value[COMMON_PATH_SIZE];

	FePath() {}
	FePath(const FePath& other)
	{
		sprintf_s(Value, other.Value);
	}

	FePath& operator=(const FePath& other)
	{
		sprintf_s(Value, other.Value);
		return *this;
	}
	inline void Set(const char* str);
};

#define SafeDelete(ptr) {if (ptr) delete ptr; }
#define SafeDeleteArray(ptr) {if (ptr) delete[] ptr; }