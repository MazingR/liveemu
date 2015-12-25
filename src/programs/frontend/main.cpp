#include <pch.hpp>

#include <rendering/modulerenderer.hpp>
#include <ui/moduleui.hpp>
#include <SDL_syswm.h>
#include <map>

#define HEAP_APPLICATION 0

uint32 FeApplication::Load(const FeApplicationInit& appInit)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
		return EFeReturnCode::Failed;
	}
	static char szWindowName[512] = "Hello World!";

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version)

	SDL_Window* window = SDL_CreateWindow(szWindowName, 100, 100, 1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE); //SDL_WINDOW_FULLSCREEN_DESKTOP
	
	if (window == nullptr)
	{
		FE_LOG("SDL_CreateWindow Error: %s", SDL_GetError());
		SDL_Quit();
		return EFeReturnCode::Failed;
	}
	
	SDL_GetWindowWMInfo(window, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;


	{
		FeCommon::FeModuleInit init;
		FE_FAILEDRETURN(CreateAndLoadModule<FeRendering::FeModuleRenderResourcesHandler>(init));
	}
	{
		FeRendering::FeModuleRenderingInit init;
		
		init.WindowsCmdShow = appInit.WindowsCmdShow;
		init.WindowsInstance = (HINSTANCE)appInit.WindowsInstance;
		init.WindowHandle = hwnd;

		auto pModule = CreateModule<FeRendering::FeModuleRendering>();
		FE_FAILEDRETURN(pModule->Load(&init));
	}
	{
		FeUi::FeModuleUiInit init;
		FE_FAILEDRETURN(CreateAndLoadModule<FeUi::FeModuleUi>(init));
	}

	return EFeReturnCode::Success;
}
uint32 FeApplication::Unload()
{
	for (ModulesMapIt it = Modules.begin(); it != Modules.end(); ++it)
		FE_FAILEDRETURN(it->second->Unload());

	return EFeReturnCode::Success;
}	  
uint32 FeApplication::Run()
{
	SDL_Event e;
	bool bQuit = false;
	uint32 iTicks = SDL_GetTicks();

	FeDt fDt;
	ZeroMemory(&fDt, sizeof(fDt));

	while (!bQuit)
	{
		uint32 iPreviousTicks = iTicks;
		iTicks = SDL_GetTicks();
		fDt.TotalMilliseconds = iTicks - iPreviousTicks;
		fDt.TotalSeconds = fDt.TotalMilliseconds / 1000.0f;

		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_KEYDOWN)
			{
				switch (e.key.keysym.sym){
				case SDLK_F1:
					GetModule<FeRendering::FeModuleRendering>()->SwitchDebugRenderTextMode();
					break;
				case SDLK_F2:
					unitTest();
					break;
				}
			}

			if (e.type == SDL_QUIT)
				bQuit = true;
		 }

		for (ModulesMapIt it = Modules.begin(); it != Modules.end(); ++it)
		{
			FE_FAILEDRETURN(it->second->Update(fDt));
		}
		
		// Limit framerate (60fps)
		uint32 iFrameTicks = SDL_GetTicks() - iTicks;
		fDt.TotalCpuWaited = 16 - iFrameTicks;

		if (fDt.TotalCpuWaited>0)
			SDL_Delay(fDt.TotalCpuWaited);
	}

	return EFeReturnCode::Success;
}

//int _tmain(int argc, _TCHAR* argv[])
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	FeCommon::FeMemoryManager::StaticInstance.CreateHeapMBytes(16, "SDL2");
	FeCommon::FeMemoryManager::StaticInstance.CreateHeapMBytes(32, "Renderer");

	FeApplication& app = FeApplication::StaticInstance;
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
