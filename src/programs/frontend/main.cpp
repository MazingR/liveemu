#include <pch.hpp>

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

uint32 file_read(const char* filename, void** outputBuff) {
	SDL_RWops *rw = SDL_RWFromFile(filename, "r");
	
	if (rw == NULL) 
		return EFeReturnCode::File_OpenFailed;

	Sint64 res_size = SDL_RWsize(rw);
	*outputBuff = (char*)FE_ALLOCATE(res_size + 1, 0);

	Sint64 nb_read_total = 0, nb_read = 1;
	char* buf = (char*)*outputBuff;

	while (nb_read_total < res_size && nb_read != 0) {
		nb_read = SDL_RWread(rw, buf, 1, (res_size - nb_read_total));
		nb_read_total += nb_read;
		buf += nb_read;
	}
	SDL_RWclose(rw);
	if (nb_read_total != res_size) {
		free(*outputBuff);
		return EFeReturnCode::File_ReadFailed;
	}

	((char*)*outputBuff)[nb_read_total] = '\0';

	return EFeReturnCode::Success;
}

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

	static char szWindowName[512] = "Hello World!";

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
			if (e.type == SDL_QUIT || e.type == SDL_KEYDOWN)
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
		for (uint32 i = 0; i<iFindTimes; ++i)
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
			
		for (uint32 i = 0; i < iFindTimes; ++i)
			it = map.find(iKeyToFind);
		PERF_PRINT_SET(time, "find");
		FE_LOG("\tFound value '%s' ?= %s ", valueToFind->Name, it->second->Name);
	}
	
}


void test2()
{
	FeCommon::FeTArray<char> testArray(50 * (1000 * 1000), 0);
	FeCommon::FeTArray<char> testArray1(10 * (1000 * 1000), 0);
	testArray.Clear();
	testArray1.Clear();
	FeCommon::FeTArray<char> testArray2(20 * (1000 * 1000), 0);
	testArray2.Clear();
}

//int _tmain(int argc, _TCHAR* argv[])
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	const int iCount = 10;
	FeCommon::FeMemoryManager::StaticInstance.CreateHeapMBytes(5 * iCount);

	FeApplication app;
	FeApplicationInit init;

	init.WindowsCmdLine			= lpCmdLine;
	init.WindowsCmdShow			= nCmdShow;
	init.WindowsInstance		= hInstance;
	init.WindowsPrevInstance	= hPrevInstance;

	FE_FAILEDRETURN(app.Load(init));

	//std::string szPaths[4096];
	char* szBasePath = SDL_GetBasePath();

	void* ptrs[iCount];

	for (int i = 0; i < iCount; ++i)
	{
		std::string szPath = getResourcePath() + "image.jpg";
		file_read(szPath.c_str(), &ptrs[i]);
	}

	FE_FAILEDRETURN(app.Run());
	FE_FAILEDRETURN(app.Unload());

	return 0;
}