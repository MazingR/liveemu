#pragma once 

#include <common.hpp>
#include <typeinfo.h>
#include <map>

struct FeModuleInit
{};

class FeModule
{
public:
	virtual uint32 Load(const FeModuleInit*) = 0;
	virtual uint32 Unload() = 0;
	virtual uint32 Update(const FeDt& fDt) = 0;
};

struct FeApplicationInit
{
public:
	void*		WindowsInstance;
	void*		WindowsPrevInstance;
	wchar_t*	WindowsCmdLine;
	int			WindowsCmdShow;
};
class FeApplication
{
public:
	static FeApplication StaticInstance;

	typedef std::map<size_t, FeModule*> ModulesMap;
	typedef ModulesMap::iterator ModulesMapIt;

	uint32 Load(const FeApplicationInit&);
	uint32 Unload();
	uint32 Run();

	template<class T>
	T* CreateModule()
	{
		T* pModule = FE_NEW(T, 0);
		Modules[GetModuleId<T>()] = pModule;
		return pModule;
	}

	template<class T>
	uint32 CreateAndLoadModule(const FeModuleInit& init)
	{
		T* pModule = CreateModule<T>();
		return pModule->Load(&init);
	}
	template<class T>
	T* GetModule()
	{
		return (T*)Modules[GetModuleId<T>()];
	}
	template<class T>
	size_t GetModuleId()
	{
		static uint32 iTypeHash = typeid(T).hash_code();
		return iTypeHash;
	}
private:
	ModulesMap Modules;
};
