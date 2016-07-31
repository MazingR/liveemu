#pragma once 

#include <common.hpp>

template<typename T>
struct FeTPtr
{
public:
	T* Ptr;
	bool Owner : 1;

	FeTPtr() : Ptr(nullptr) {}
	FeTPtr(T* _Ptr) : Ptr(_Ptr), Owner(false){}

	~FeTPtr()
	{
		Delete();
	}

	T* operator->() {
		return Ptr;
	}

	T* New()
	{
		Delete();
		Ptr = FE_NEW(T, 1);
		Owner = true;

		return Ptr;
	}
	bool IsValid()
	{
		return Ptr != nullptrptr;
	}
	void Delete()
	{
		if (Ptr && Owner)
		{
			FE_DELETE(T, Ptr, 1);
		}
	}
};

