#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include "mainwindow.hpp"

#include <pch.hpp>

#define USE_LIMIT_FPS 1
#define FPS_LIMIT 60
#define HEAP_APPLICATION 0

uint32 FeApplication::Load(const FeApplicationInit& appInit)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
		return FeEReturnCode::Failed;
	}
	static char szWindowName[512] = "Hello World!";

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	
	{
		FeModuleInit init;
		FE_FAILEDRETURN(CreateAndLoadModule<FeModuleFilesManager>(init));
	}
	{
		FeModuleRenderingInit init;
		
		init.WindowsCmdShow = appInit.WindowsCmdShow;
		init.WindowsInstance = (HINSTANCE)appInit.WindowsInstance;
		init.Width = 1280;
		init.Height = 720;

		SDL_Window* window = SDL_CreateWindow(szWindowName, 100, 100, init.Width, init.Height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE); //SDL_WINDOW_FULLSCREEN_DESKTOP
		if (window == nullptr)
		{
			FE_LOG("SDL_CreateWindow Error: %s", SDL_GetError());
			SDL_Quit();
			return FeEReturnCode::Failed;
		}

		SDL_GetWindowWMInfo(window, &wmInfo);
		init.WindowHandle = wmInfo.info.win.window;

		auto pModule = CreateModule<FeModuleRendering>();
		FE_FAILEDRETURN(pModule->Load(&init));
	}
	{
		FeModuleInit init;
		FE_FAILEDRETURN(CreateAndLoadModule<FeModuleRenderResourcesHandler>(init));
	}
	{
		FeModuleUiInit init;
		FE_FAILEDRETURN(CreateAndLoadModule<FeModuleUi>(init));
	}
	//{
	//	FeModuleInit init;
	//	FE_FAILEDRETURN(CreateAndLoadModule<FeModuleFontsHandler>(init));
	//}

	return FeEReturnCode::Success;
}
uint32 FeApplication::Unload()
{
	for (ModulesMapIt it = Modules.begin(); it != Modules.end(); ++it)
		FE_FAILEDRETURN(it->second->Unload());

	return FeEReturnCode::Success;
}	  
uint32 FeApplication::Run()
{
	SDL_Event e;
	bool bQuit = false;
	uint32 iTicks = SDL_GetTicks();
	uint32 iMaxPolledEvents = 1;

	FeDt fDt;
	ZeroMemory(&fDt, sizeof(fDt));

	while (!bQuit)
	{
		uint32 iPreviousTicks = iTicks;
		uint32 iPolledEvents = 0;

		iTicks = SDL_GetTicks();
		fDt.TotalMilliseconds = iTicks - iPreviousTicks;
		fDt.TotalSeconds = fDt.TotalMilliseconds / 1000.0f;

		while (iPolledEvents++<iMaxPolledEvents && SDL_PollEvent(&e))
		{
			if (e.type == SDL_KEYDOWN)
			{
				switch (e.key.keysym.sym){
				case SDLK_F1:
					GetModule<FeModuleRendering>()->SwitchDebugRenderTextMode();
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
#if USE_LIMIT_FPS
		// Limit framerate (60fps)
		uint32 iFrameTicks = SDL_GetTicks() - iTicks;
		fDt.TotalCpuWaited = (1000 / FPS_LIMIT) - iFrameTicks;

		if (fDt.TotalCpuWaited>0)
			SDL_Delay(fDt.TotalCpuWaited);
#endif
	}

	return FeEReturnCode::Success;
}

//int _tmain(int argc, _TCHAR* argv[])
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	Q_INIT_RESOURCE(application);

	char argv[1][1];
	int argc = 0;

	QApplication app(argc, argv);
	QCoreApplication::setOrganizationName("QtProject");
	QCoreApplication::setApplicationName("Application Example");
	QCoreApplication::setApplicationVersion(QT_VERSION_STR);
	QCommandLineParser parser;
	parser.setApplicationDescription(QCoreApplication::applicationName());
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument("file", "The file to open.");
	parser.process(app);

	MainWindow mainWin;
	if (!parser.positionalArguments().isEmpty())
		mainWin.loadFile(parser.positionalArguments().first());
	mainWin.show();
	return app.exec();

	//FeMemoryManager::StaticInstance.CreateHeapMBytes(16, "SDL2");
	//FeMemoryManager::StaticInstance.CreateHeapMBytes(31, "Render");
	//FeMemoryManager::StaticInstance.CreateHeapMBytes(32, "Ui");
	//FeFileTools::SetRootDir("../data");

	//FeApplication& app = FeApplication::StaticInstance;
	//FeApplicationInit init;

	//init.WindowsCmdLine			= lpCmdLine;
	//init.WindowsCmdShow			= nCmdShow;
	//init.WindowsInstance		= hInstance;
	//init.WindowsPrevInstance	= hPrevInstance;

	//FE_FAILEDRETURN(app.Load(init));
	//FE_FAILEDRETURN(app.Run());
	//FE_FAILEDRETURN(app.Unload());

	return 0;
}
