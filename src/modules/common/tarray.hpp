#pragma once 

#include <common.hpp>

template <class T, class U = uint32>
class FeTArray
{
public:
	typedef T*		Iterator;
	Iterator			Begin()			{ return BaseAdress; }
	const Iterator		Begin() const	{ return BaseAdress; }
	
	Iterator			End()			{ return Begin() + GetSize(); }
	const Iterator		End() const		{ return Begin() + GetSize(); }
	
	Iterator			begin()			{ return Begin(); }
	const Iterator		begin() const	{ return Begin(); }

	Iterator			end()			{ return End(); }
	const Iterator		end() const		{ return End(); }

	bool 	    IsEmpty()	const 		{ return (Size == 0); }
	U			GetSize()	const		{ return Size; }

	FeTArray(U _iSize = 0, THeapId iHeapId = DEFAULT_HEAP)
		: BaseAdress(NULL)
		, Size(0)
		, EffectiveSize(0)
		, HeapId(iHeapId)
	{
		if (_iSize>0)
			Reserve(_iSize);
	}
	FeTArray(const FeTArray& _copy, THeapId iHeapId = DEFAULT_HEAP)
		: BaseAdress(NULL)
		, HeapId(iHeapId)
	{
		Size = _copy.Size;
		EffectiveSize = Size;
		BaseAdress = FE_NEW_ARRAY(T, Size, HeapId);

		for (U i = 0; i < Size; i++)
		{
			BaseAdress[i] = _copy.BaseAdress[i];
		}
	}
	~FeTArray()
	{
		Free();
	}

	const T&	operator[](U i) const
	{
		return GetAt(i);
	}
	T&			operator[](U i)
	{
		return GetAt(i);
	}
	bool		operator ==(const FeTArray& _array) const
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
	FeTArray&	operator =(const FeTArray& _copy)
	{
		Size = _copy.Size;
		if (Size>EffectiveSize)
		{
			if (BaseAdress != NULL)
			{
				FE_DELETE_ARRAY(T, BaseAdress, EffectiveSize, HeapId);
			}
			EffectiveSize = Size;
			BaseAdress = FE_NEW_ARRAY(T, EffectiveSize, HeapId);
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
	const T&	GetAt(U i) const
	{
		return BaseAdress[i];
	}
	T&			GetAt(U i)
	{
		return BaseAdress[i];
	}
	void		Reserve(U _iSize)
	{
		if (_iSize>EffectiveSize)
		{
			T* pNewBase = FE_NEW_ARRAY(T, _iSize, HeapId);
			if (Size>0)
			{
				for (U i = 0; i < (_iSize<Size ? _iSize : Size); i++)
				{
					pNewBase[i] = BaseAdress[i];
				}
			}
			if (BaseAdress != NULL)
			{
				FE_DELETE_ARRAY(T, BaseAdress, EffectiveSize, HeapId);
			}
			BaseAdress = pNewBase;
			EffectiveSize = _iSize;
		}
	}
	void		Resize(U _iSize)
	{
		T* pNewBase = NULL;
			
		if (_iSize>0)
			pNewBase = FE_NEW_ARRAY(T, _iSize, HeapId);

		if (_iSize>0 && Size>0)
		{
			for (U i = 0; i < FeMath::Min(_iSize, Size); i++)
			{
				pNewBase[i] = BaseAdress[i];
			}
		}
		if (BaseAdress != NULL)
		{
			FE_DELETE_ARRAY(T, BaseAdress, EffectiveSize, HeapId);
		}
		BaseAdress = pNewBase;
		EffectiveSize = _iSize;
		Size = _iSize;
	}
	void		Resize(U _iSize, const T& _kDefaultValue)
	{
		T* pNewBase = NULL;
		if (_iSize>0)
			pNewBase = FE_NEW_ARRAY(T, _iSize, HeapId);
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
			FE_DELETE_ARRAY(T, BaseAdress, EffectiveSize, HeapId);
		}
		BaseAdress = pNewBase;
		EffectiveSize = _iSize;
		Size = _iSize;
	}
	T&			Add()
	{
		const U uiMaxValue = U(~U(0));
		const U uiUpperLimitMulByTwo = (uiMaxValue >> 1) + 1;
			
		FE_ASSERT(Size<uiMaxValue, "FeTArray capacity is full already");

		Size++;
		U iPreviousSize = EffectiveSize;

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

			T* pNewBase = FE_NEW_ARRAY(T, EffectiveSize, HeapId);
			for (U i = 0; i < Size - 1; i++)
			{
				pNewBase[i] = BaseAdress[i];
			}

			if (BaseAdress != NULL)
			{
				FE_DELETE_ARRAY(T, BaseAdress, iPreviousSize, HeapId);
			}
			BaseAdress = pNewBase;
		}
		return BaseAdress[Size - 1];
	}
	void		Add(const T& element)
	{
		T& baseelem = Add();
		baseelem = element;
	}
	void		Add(const T* _pElement, U _num)
	{
		Reserve(Size + _num);
		for (U i = 0; i <_num; ++i)
		{
			T& baseelem = Add();
			baseelem = _pElement[i];
		}
	}
	T			PopBack()
	{
		T& back = BaseAdress[Size - 1];
		Size--;
		return back;
	}
	T&			Back()
	{
		return BaseAdress[Size - 1];
	}
	static U	ErrorIndex(){ return U(-1); }
	U			Find(const T& element) const
	{
		for (U i = 0; i < Size; i++)
		{
			if (BaseAdress[i] == element)
				return i;
		}
		return ErrorIndex();
	}
	U			SortedFind(const T& element) const
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
	bool		InsertAt(U index, const T& element)
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
	T			RemoveAt(U index)
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
	T			RemoveAtNoOrdering(U index)
	{
		T old = BaseAdress[index];
		Size -= 1;
		if (Size>0)
		{
			BaseAdress[index] = BaseAdress[Size];
		}
		return old;
	}
	T			SetAt(U i, const T& element)
	{
		T old = BaseAdress[i];
		BaseAdress[i] = element;
		return old;
	}
	void		Clear(U _size = 0) //nothing destroyed, capacity is unchanged, only size is 0
	{
		Size = _size;
	}
	void		Free()
	{
		if (BaseAdress != NULL)
		{
			FE_DELETE_ARRAY(T, BaseAdress, EffectiveSize, HeapId);
			BaseAdress = NULL;
		}
		Size = 0;
		EffectiveSize = 0;
	}
	void		SetZeroMemory()
	{
		memset(BaseAdress, 0, EffectiveSize*sizeof(T));
	}
	T*			GetBaseAdress() { return BaseAdress; }
	void		SetHeapId(THeapId iHeapId)
	{
		if (iHeapId != HeapId)
		{
			T* newBaseAdress = FE_NEW_ARRAY(T, EffectiveSize, iHeapId);
				
			for (U i = 0; i < Size; i++)
			{
				newBaseAdress[i] = BaseAdress[i];
			}
			FE_DELETE_ARRAY(T, BaseAdress, EffectiveSize, HeapId);
			BaseAdress = newBaseAdress;
			HeapId = iHeapId;
		}
	}
private:
	T*	BaseAdress;
	U	Size;
	U	EffectiveSize;
	THeapId HeapId;
};
	
template <class TKey, class TValue, class U=uint32>
class FeTMap
{
#define FETASH_COMPARATOR_ENTRY_KEY(a,b) SGLIB_NUMERIC_COMPARATOR(a.Key, b)
#define FETASH_COMPARATOR_ENTRY_ENTRY(a,b) SGLIB_NUMERIC_COMPARATOR(a.Key, b.Key)

public:
	struct KeyEntry
	{
		TKey	Key;
		U		ValueIndex;
	};

	void Reserve(U size)
	{
		Keys.Reserve(size);
		Values.Reserve(size);
	}
	void Remove(TKey key)
	{
		KeyEntry keyEntry;

		U index = Find(key);

		if (index != Keys.ErrorIndex())
		{

		}
	}
	TValue& Add(TKey key)
	{
		return DoAdd(key, true);
	}
	TValue& AddNoSort(TKey key)
	{
		return DoAdd(key, false);
	}
	void AddNoSort(TKey key, const TValue& value)
	{
		DoAdd(key, value, false);
	}
	void Add(TKey key, const TValue& value)
	{
		DoAdd(key, value, true);
	}

	TValue& operator[](TKey key)
	{
		U index = Find(key);

		FE_ASSERT(index != Values.ErrorIndex(), "FeTHash find failed !")

		return Values[index];
	}
		
	U FindKey(TKey key)
	{
		int iFound = 0;
		U iFoundIdx = 0;

		SGLIB_ARRAY_BINARY_SEARCH(U, Keys.GetBaseAdress(), 0, Keys.GetSize() - 1, key, FETASH_COMPARATOR_ENTRY_KEY, iFound, iFoundIdx);

		return iFound ? iFoundIdx : Keys.ErrorIndex();
	}
	U Find(TKey key)
	{
		for (U i = 0; i < Keys.GetSize(); ++i)
		{
			if (Keys[i].Key == key)
				return Keys[i].ValueIndex;
		}
		return Keys.ErrorIndex();

		//int iFound = 0;
		//U iFoundIdx = 0;

		//SGLIB_ARRAY_BINARY_SEARCH(U, Keys.GetBaseAdress(), 0, Keys.GetSize() - 1, key, FETASH_COMPARATOR_ENTRY_KEY, iFound, iFoundIdx);
		//
		//return iFound ? Keys[iFoundIdx].ValueIndex : Keys.ErrorIndex();
	}
	void Sort()
	{
		//SGLIB_ARRAY_SINGLE_QUICK_SORT(KeyEntry, Keys.GetBaseAdress(), Keys.GetSize(), FETASH_COMPARATOR_ENTRY_ENTRY);
	}
	TValue& GetValueAt(U idx)
	{
		return Values[idx];
	}
	TKey& GetKeyAt(U idx)
	{
		return Keys[idx];
	}
	U GetSize()
	{
		return Keys.GetSize();
	}
private:
	void DoAdd(TKey key, const TValue& value, bool bSort)
	{
		FE_ASSERT(Find(key) == Keys.ErrorIndex(), "FeTHash Add key already present !");

		Values.Add(value);

		KeyEntry& keyEntry = Keys.Add();
		keyEntry.Key = key;
		keyEntry.ValueIndex = Values.GetSize() - 1;

		if (bSort)
			Sort();
	}
	TValue& DoAdd(TKey key, bool bSort)
	{
		FE_ASSERT(Find(key) == Keys.ErrorIndex(), "FeTHash Add key already present !");

		TValue& newValue = Values.Add();

		KeyEntry& keyEntry = Keys.Add();
		keyEntry.Key = key;
		keyEntry.ValueIndex = Values.GetSize() - 1;

		if (bSort)
			Sort();

		return newValue;
	}
		
	FeTArray<KeyEntry, U> Keys;
	FeTArray<TValue, U> Values;
};