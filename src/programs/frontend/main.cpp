#include <pch.hpp>

#include <rendering/modulerenderer.hpp>
#include <ui/module.hpp>
#include <SDL_syswm.h>

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
			
			if (e.type == SDL_KEYDOWN)
				unitTest();

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

//int _tmain(int argc, _TCHAR* argv[])
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	FeCommon::FeMemoryManager::StaticInstance.CreateHeapMBytes(16, "SDL2");
	FeCommon::FeMemoryManager::StaticInstance.CreateHeapMBytes(32, "Test");

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
