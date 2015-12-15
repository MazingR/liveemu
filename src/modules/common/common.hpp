#pragma once 

#include <stdio.h>

typedef short int uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

namespace EFeReturnCode
{
	enum Type
	{
		Success = 0,
		Failed,
		Canceled,

		Rendering_CreateShaderFailed,
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

#define FE_ALLOCATE_TYPED_ARRAY(type, size) (type*)malloc((size_t)size*sizeof(type))
#define FE_FREE_ARRAY(ptr) free(ptr)

typedef struct SDL_Rect
{
	int x, y;
	int w, h;
} FeRect;

namespace FeCommon
{
	struct FeModuleInit
	{};

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
	class FeModule
	{
	public:
		virtual uint32 Load(const ::FeCommon::FeModuleInit*) = 0;
		virtual uint32 Unload() = 0;
		virtual uint32 Update() = 0;
	};

}