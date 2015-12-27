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
#define COMMON_STR_SIZE 128
#define COMMON_PATH_SIZE 512

#ifdef DEBUG
#define CONFIGSTR "Debug"
#elif defined(MASTER)
#define CONFIGSTR "Master"
#else
#define CONFIGSTR "Release"
#endif

#undef DEFAULT_HEAP
#define DEFAULT_HEAP (unsigned int)-1

#include <newhook.hpp>
#include <mallochook.hpp>

typedef unsigned int THeapId;
#define INIT_HEAP_ID ((THeapId)-1)

typedef short unsigned int uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef short int int16;
typedef int int32;
typedef long long int64;

namespace EFeReturnCode
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
	};
};

int __cdecl vs_printf(const char *format, ...);

#define FE_FAILED(a) (a!=EFeReturnCode::Success)
#define FE_FAILEDRETURN(a) { uint32 ___iResult = (a); { if FE_FAILED(___iResult) return ___iResult; } }
#define FE_LOG(fmt, ...) vs_printf(fmt, __VA_ARGS__);vs_printf("\n");

#ifdef DEBUG
	#define FE_ASSERT(condition, fmt, ...) { if (!(condition)) { FE_LOG(fmt, __VA_ARGS__);__debugbreak();  } }
#else
	#define FE_ASSERT(condition, fmt, ...) 
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
void FeDelete(void* ptr, THeapId iHeapId = DEFAULT_HEAP)
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

namespace FeCommon
{
}

#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/Vec.h>
#include <gmtl/VecOps.h>
#include <gmtl/Generate.h>

typedef gmtl::Vec3f FeVector3;
typedef gmtl::Matrix33f FeMatrix3;
typedef gmtl::EulerAngleXYZf FeRotation;
typedef gmtl::Vec4f FeVector4;
typedef gmtl::Matrix44f FeMatrix4;

struct FeDt
{
	float	TotalSeconds;
	uint32	TotalMilliseconds;
	int		TotalCpuWaited;
};

namespace FeMath
{
	template <typename T> static inline T		Min(const T& a, const T& b)
	{
		return(a < b ? a : b);
	}
	template <typename T> static inline T		Max(const T& a, const T& b)
	{
		return(a > b ? a : b);
	}
	template <typename T> static inline T       Clamp(const T& a, const T& min, const T& max)
	{
		return Min<T>(max, Max<T>(min, a));
	}
	template <typename T> static inline T		Abs(const T& val)
	{
		return(val < 0 ? -val : val);
	}
	template <typename T> static inline T		Range(const T& min, const T& max, const T& val)
	{
		return (max > val ? (min < val ? val : min) : max);
	}
	template <typename T> static inline float	Ratio(const T& min, const T& max, const T& val)
	{
		if (val <= min) return 0.0f;
		if (val >= max) return 1.0f;

		return float(val - min) * (1.0f / float(max - min));
	}	
};
