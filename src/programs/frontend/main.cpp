#include <common/common.hpp>
#include <common/string.hpp>
#include <common/tarray.hpp>

#include <rendering/module.hpp>
#include <ui/module.hpp>

#include <SDL_syswm.h>
#include "respath.hpp"

struct FeApplicationInit
{
public:
	HINSTANCE	WindowsInstance;
	HINSTANCE	WindowsPrevInstance;
	wchar_t*	WindowsCmdLine;
	int			WindowsCmdShow;
};
class FeApplication
{
private:

	static const uint32 MaxModules = 16;
	uint32				LoadedModules;

	FeCommon::FeModule* Modules[MaxModules];
public:
	uint32 Load(const FeApplicationInit&);
	uint32 Unload();
	uint32 Run();
};

uint32 FeApplication::Load(const FeApplicationInit& appInit)
{
	memset(Modules, 0, MaxModules*sizeof( void*));
	LoadedModules = 0;

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
		return EFeReturnCode::Failed;
	}
	FE_LOG("Resource path is: %s", getResourcePath());

	char szWindowName[512] = "Hello World!";

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version)

	SDL_Window* window = SDL_CreateWindow(szWindowName, 100, 100, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE); //SDL_WINDOW_FULLSCREEN_DESKTOP
	
	if (window == nullptr)
	{
		FE_LOG("SDL_CreateWindow Error: %s", SDL_GetError());
		SDL_Quit();
		return EFeReturnCode::Failed;
	}
	
	SDL_GetWindowWMInfo(window, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;

	{
		FeRendering::FeModuleRenderingInit init;
		
		init.WindowsCmdShow = appInit.WindowsCmdShow;
		init.WindowsInstance = appInit.WindowsInstance;
		init.WindowHandle = hwnd;

		auto pModuleRendering = new FeRendering::FeModuleRendering();
		FE_FAILEDRETURN(pModuleRendering->Load(&init));
		
		Modules[0] = pModuleRendering;
		LoadedModules++;
	}
	{
		FeUi::FeModuleUiInit init;

		auto pModuleUi = new FeUi::FeModuleUi();
		FE_FAILEDRETURN(pModuleUi->Load(&init));

		Modules[1] = pModuleUi;
		LoadedModules++;
	}

	return EFeReturnCode::Success;
}
uint32 FeApplication::Unload()
{
	for (uint32 i = 0; i < LoadedModules; ++i)
	{
		FE_FAILEDRETURN(Modules[i]->Unload());
	}
	return EFeReturnCode::Success;
}	  
uint32 FeApplication::Run()
{
	SDL_Event e;
	bool bQuit = false;
	
	while (!bQuit)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
				bQuit = true;
		 }
		
		for (uint32 i = 0; i < LoadedModules; ++i)
		{
			FE_FAILEDRETURN(Modules[i]->Update());
		}
	}

	return EFeReturnCode::Success;
}
struct TestValue
{
	char Name[32];
};

#define PERF_RESET(time) time = std::clock();

#define PERF_PRINT_SET(time, msg) {\
	double ___duration = (std::clock() - time) / (double)CLOCKS_PER_SEC; \
	FE_LOG("\tPerf (%s) = %4.8f (s)", msg, ___duration); \
	PERF_RESET(time) }

#include <map>

void test1()
{
	srand(time(NULL));
	std::clock_t time;
	double duration;
	
	uint32 iValuesCount = 2 << 22;
	FE_LOG("Unit test map with %d elements", iValuesCount);

	PERF_RESET(time);
	
	uint32* testKeys = new uint32[iValuesCount];
	TestValue* testValues = new TestValue[iValuesCount];

	for (uint32 i = 0; i < iValuesCount; ++i)
	{
		uint32 iKey = rand() % 1000;

		testKeys[i] = iKey;
		sprintf_s(testValues[i].Name, "value_%d", iKey);
	}
	uint32 iIdxOfFind = 2 << 16;
	uint32 iFindTimes = 2 << 16;

	uint32 iKeyToFind = testKeys[iIdxOfFind];
	TestValue* valueToFind = &testValues[iIdxOfFind];
	
	PERF_PRINT_SET(time, "Generate data");

	{
		FeCommon::FeTMap<uint32, TestValue*> map;

		map.Reserve(iValuesCount);
		for (uint32 i = 0; i < iValuesCount; ++i)
		{
			map.AddNoSort(testKeys[i], &testValues[i]);
		}
		
		FE_LOG("FeTMap");
		PERF_PRINT_SET(time, "inject data");
		uint32 iFoundIdx = 0;
		for(int i=0;i<iFindTimes;++i)
			iFoundIdx = map.Find(iKeyToFind);
		PERF_PRINT_SET(time, "find");
		FE_LOG("\tFound value '%s' ?= %s ", valueToFind->Name, map.GetValueAt(iFoundIdx)->Name);
	}
	{
		std::map<uint32, TestValue*> map;
		
		for (uint32 i = 0; i < iValuesCount; ++i)
		{
			map[testKeys[i]] = &testValues[i];
		}
		
		FE_LOG("StdMap");

		PERF_PRINT_SET(time, "inject data");
		std::map<uint32, TestValue*>::iterator it;
			
		for (int i = 0; i < iFindTimes; ++i)
			it = map.find(iKeyToFind);
		PERF_PRINT_SET(time, "find");
		FE_LOG("\tFound value '%s' ?= %s ", valueToFind->Name, it->second->Name);
	}
	
}


void test2()
{

}

//int _tmain(int argc, _TCHAR* argv[])
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	test2();

	FeApplication app;
	FeApplicationInit init;

	init.WindowsCmdLine			= lpCmdLine;
	init.WindowsCmdShow			= nCmdShow;
	init.WindowsInstance		= hInstance;
	init.WindowsPrevInstance	= hPrevInstance;

	FE_FAILEDRETURN(app.Load(init));
	FE_FAILEDRETURN(app.Run());
	FE_FAILEDRETURN(app.Unload());

	return 0;
}