#pragma once 

#include <sglib.h>
#include <stdio.h>
#include <tchar.h>

#include <SDL.h>

typedef unsigned int uint32;
typedef short int uint16;

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

#define FE_FAILED(a) (a!=EFeReturnCode::Success)
#define FE_FAILEDRETURN(a) { uint32 ___iResult = (a); { if FE_FAILED(___iResult) return ___iResult; } }
#define FE_LOG(fmt, ...) printf(fmt, __VA_ARGS__)

#ifdef DEBUG
#define FE_ASSERT(condition, fmt, ...) { if (!(condition)) { FE_LOG(fmt, __VA_ARGS__);__debugbreak();  } }
#else
#define FE_ASSERT(condition, fmt, ...) 
#endif

#define FE_ALLOCATE_TYPED_ARRAY(type, size) (type*)malloc(size*sizeof(type))
#define FE_FREE_ARRAY(ptr) free(ptr)

typedef SDL_Rect FeRect;

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
		template<typename T> static inline  T       Clamp(const T& a, const T& min, const T& max)
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

	template <class T, class U=uint32>
	class FeTArray
	{
	public:
		bool 	    IsEmpty()	const 				{ return (Size == 0); }
		U			GetSize()	const				{ return Size; }

		FeTArray(U _iSize = 0)
			: BaseAdress(NULL)
			, Size(0)
			, EffectiveSize(0)
		{
			if (_iSize>0)
				Reserve(_iSize);
		}
		FeTArray(const FeTArray& _copy)
			: BaseAdress(NULL)
		{
			Size = _copy.Size;
			EffectiveSize = Size;
			BaseAdress = FE_ALLOCATE_TYPED_ARRAY(T, Size);

			for (U i = 0; i < Size; i++)
			{
				BaseAdress[i] = _copy.BaseAdress[i];
			}
		}
		~FeTArray()
		{
			Free();
		}

		bool operator ==(const FeTArray& _array) const
		{
			U s = GetSize();
			if (_array.GetSize() != s)
				return false;

			for (U i = 0; i < s; ++i)
			{
				if (_array[i] != (*this)[i])
					return false;
			}

			return true;
		}

		const T& operator[](U i) const
		{
			return GetAt(i);
		}
		
		T& operator[](U i)
		{
			return GetAt(i);
		}
		
		FeTArray& operator =(const FeTArray& _copy)
		{
			Size = _copy.Size;
			if (Size>EffectiveSize)
			{
				if (BaseAdress != NULL)
				{
					FE_FREE_ARRAY(BaseAdress);
				}
				EffectiveSize = Size;
				BaseAdress = FE_ALLOCATE_TYPED_ARRAY(T, EffectiveSize);
			}
			if (Size>0)
			{
				for (U i = 0; i < Size; i++)
				{
					BaseAdress[i] = _copy.BaseAdress[i];
				}
			}
			return *this;
		}
		
		const T& GetAt(U i) const
		{
			return BaseAdress[i];
		}
		
		T& GetAt(U i)
		{
			return BaseAdress[i];
		}

		void Reserve(U _iSize)
		{
			if (_iSize>EffectiveSize)
			{
				T* pNewBase = FE_ALLOCATE_TYPED_ARRAY(T, _iSize);
				if (Size>0)
				{
					for (U i = 0; i < FeMath::Min(_iSize, Size); i++)
					{
						pNewBase[i] = BaseAdress[i];
					}
				}
				if (BaseAdress != NULL)
				{
					FE_FREE_ARRAY(BaseAdress);
				}
				BaseAdress = pNewBase;
				EffectiveSize = _iSize;
			}
		}

		void Resize(U _iSize)
		{
			T* pNewBase = NULL;
			
			if (_iSize>0)
				pNewBase = FE_ALLOCATE_TYPED_ARRAY(T, _iSize);

			if (_iSize>0 && Size>0)
			{
				for (U i = 0; i < FeMath::Min(_iSize, Size); i++)
				{
					pNewBase[i] = BaseAdress[i];
				}
			}
			if (BaseAdress != NULL)
			{
				FE_FREE_ARRAY(BaseAdress);
			}
			BaseAdress = pNewBase;
			EffectiveSize = _iSize;
			Size = _iSize;
		}

		void Resize(U _iSize, const T& _kDefaultValue)
		{
			T* pNewBase = NULL;
			if (_iSize>0)
				pNewBase = FE_ALLOCATE_TYPED_ARRAY(T, _iSize);
			if (_iSize>0 && Size>0)
			{
				for (U i = 0; i < Math::Min(_iSize, Size); i++)
				{
					pNewBase[i] = BaseAdress[i];
				}
			}
			for (U i = Size; i < _iSize; i++)
			{
				pNewBase[i] = _kDefaultValue;
			}
			if (BaseAdress != NULL)
			{
				FE_FREE_ARRAY(BaseAdress);
			}
			BaseAdress = pNewBase;
			EffectiveSize = _iSize;
			Size = _iSize;
		}

		T& Add()
		{
			const U uiMaxValue = U(~U(0));
			const U uiUpperLimitMulByTwo = (uiMaxValue >> 1) + 1;
			
			FE_ASSERT(Size<uiMaxValue, "FeTArray capacity is full already");

			Size++;

			if (Size > EffectiveSize)
			{
				U oldsize = EffectiveSize;

				// U type must be unsigned !
				if (EffectiveSize >= uiUpperLimitMulByTwo)
				{
					if (EffectiveSize < uiMaxValue)
					{
						// more than half capacity is allocated and filled with data. We allocate all that's left. 
						EffectiveSize = uiMaxValue;
					}
					else
					{
						// full capacity is already allocated and filled with data !
						FE_ASSERT(false, "FeTArray capacity is full already");
					}
				}
				else
				{
					EffectiveSize = ((EffectiveSize == 0) ? 8 : EffectiveSize) * 2;
				}

				T* pNewBase = FE_ALLOCATE_TYPED_ARRAY(T, EffectiveSize);
				for (U i = 0; i < Size - 1; i++)
				{
					pNewBase[i] = BaseAdress[i];
				}

				if (BaseAdress != NULL)
				{
					FE_FREE_ARRAY(BaseAdress);
				}
				BaseAdress = pNewBase;
			}
			return BaseAdress[Size - 1];
		}

		void Add(const T& element)
		{
			T& baseelem = Add();
			baseelem = element;
		}

		void Add(const T* _pElement, U _num)
		{
			Reserve(Size + _num);
			for (U i = 0; i <_num; ++i)
			{
				T& baseelem = Add();
				baseelem = _pElement[i];
			}
		}
		
		T    PopBack()
		{
			T& back = BaseAdress[Size - 1];
			Size--;
			return back;
		}

		static U ErrorIndex(){ return U(-1); }

		U Find(const T& element) const
		{
			for (U i = 0; i < Size; i++)
			{
				if (BaseAdress[i] == element)
					return i;
			}
			return ErrorIndex();
		}
		U SortedFind(const T& element) const
		{
			U _index = ErrorIndex();;

			U uiBegin = 0;
			U uiEnd = GetSize();
			while (uiBegin != uiEnd)
			{
				const U uiMiddle = (uiBegin + uiEnd) / 2;
				const T& e = GetAt(uiMiddle);
				if (element < e)
				{
					uiEnd = uiMiddle;
				}
				else if (element == e)
				{
					_index = uiMiddle;
					break;
				}
				else
				{
					uiBegin = uiMiddle + 1;
				}
			}
			return _index;
		}

		bool InsertAt(U index, const T& element)
		{
			if (index<Size)
			{
				Add();
				if (Size>1)
				{
					for (int i = static_cast<int>(Size - 2); i >= static_cast<int>(index); i--)
					{
						BaseAdress[i + 1] = BaseAdress[i];
					}
				}
				BaseAdress[index] = element;
				return true;
			}
			return false;
		}

		T RemoveAt(U index)
		{
			T old = BaseAdress[index];
			for (U i = index; i < Size - 1; i++)
			{
				BaseAdress[i] = BaseAdress[i + 1];
			}
			Size -= 1;
			return old;
		}

		///< Remove an element by swapping last element : do not preserve order. Fast o(1)
		T RemoveAtNoOrdering(U index)
		{
			T old = BaseAdress[index];
			Size -= 1;
			if (Size>0)
			{
				BaseAdress[index] = BaseAdress[Size];
			}
			return old;
		}

		T SetAt(U i, const T& element)
		{
			T old = BaseAdress[i];
			BaseAdress[i] = element;
			return old;
		}

		void Clear(U _size = 0) //nothing destroyed, capacity is unchanged, only size is 0
		{
			Size = _size;
		}
		
		void Free()
		{
			if (BaseAdress != NULL)
			{
				FE_FREE_ARRAY(BaseAdress);
				BaseAdress = NULL;
			}
			Size = 0;
			EffectiveSize = 0;
		}

		void SetZeroMemory()
		{
			memset(BaseAdress, 0, EffectiveSize*sizeof(T));
		}
	private:
		T*	BaseAdress;
		U	Size;
		U	EffectiveSize;
	};
}