#include <SDKDDKVer.h>
#include <stdio.h>
#include <tchar.h>

#define SDL_MAIN_HANDLED
#include "respath.hpp"

typedef unsigned int uint32;

namespace EFEReturnCode
{
	enum Type
	{
		Success = 0,
		Failed
	};
};



#define FEFAILED(a) (a!=EFEReturnCode::Success)
#define FEFAILEDRETURN(a) { uint32 ___iResult = (a); { if FEFAILED(___iResult) return ___iResult; } }
#define FELOG(fmt, ...) SDL_Log(fmt, __VA_ARGS__)

class FEModule
{
public:
	virtual uint32 Load() = 0;
	virtual uint32 Unload() = 0;
	virtual uint32 Update() = 0;
};

class FEApplication
{
private:
	static const uint32 c_iMaxModules = 16;
	uint32 m_iLoadedModules;
	FEModule* m_modules[c_iMaxModules];
public:
	uint32 Load();
	uint32 Unload();
	uint32 Run();
};

class FEModuleRenderer : public FEModule
{
public:
	virtual uint32 Load() override;
	virtual uint32 Unload() override;
	virtual uint32 Update() override;

private:
	SDL_Window* win;
	SDL_Renderer* ren;
	SDL_Texture* tex;
};
uint32 FEModuleRenderer::Unload()
{
	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();

	return EFEReturnCode::Success;
}
uint32 FEModuleRenderer::Load()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
		return EFEReturnCode::Failed;
	}
	FELOG("Resource path is: %s", getResourcePath());

	win = SDL_CreateWindow("Hello World!", 100, 100, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE); //SDL_WINDOW_FULLSCREEN_DESKTOP
	if (win == nullptr)
	{
		FELOG("SDL_CreateWindow Error: %s",SDL_GetError());
		SDL_Quit();
		return EFEReturnCode::Failed;
	}

	ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == nullptr){
		SDL_DestroyWindow(win);
		FELOG("SDL_CreateRenderer Error: %s", SDL_GetError());
		SDL_Quit();
		return EFEReturnCode::Failed;
	}

	std::string imagePath = getResourcePath("Lesson1") + "lena512.bmp";
	SDL_Surface *bmp = SDL_LoadBMP(imagePath.c_str());
	if (bmp == nullptr){
		SDL_DestroyRenderer(ren);
		SDL_DestroyWindow(win);
		FELOG("SDL_LoadBMP Error: %s", SDL_GetError());
		SDL_Quit();
		return EFEReturnCode::Failed;
	}

	tex = SDL_CreateTextureFromSurface(ren, bmp);
	SDL_FreeSurface(bmp);

	if (tex == nullptr){
		SDL_DestroyRenderer(ren);
		SDL_DestroyWindow(win);
		FELOG("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
		SDL_Quit();
		return EFEReturnCode::Failed;
	}

	return EFEReturnCode::Success;
}
uint32 FEModuleRenderer::Update()
{
	//First clear the renderer
	SDL_RenderClear(ren);
	//Draw the texture
	SDL_RenderCopy(ren, tex, NULL, NULL);
	//Update the screen
	SDL_RenderPresent(ren);
	
	//Take a quick break after all that hard work
	//SDL_Delay(1000);

	return EFEReturnCode::Success;
}

uint32 FEApplication::Load()
{
	memset(m_modules, 0, c_iMaxModules*sizeof(FEModule*));
	m_iLoadedModules = 0;

	m_modules[0] = new FEModuleRenderer;

	for (uint32 i = 0; i < c_iMaxModules; ++i)
	{
		if (m_modules[i] != NULL)
		{
			FEFAILEDRETURN(m_modules[i]->Load());
			m_iLoadedModules++;
		}
	}

	return EFEReturnCode::Success;
}
uint32 FEApplication::Unload()
{
	for (uint32 i = 0; i < m_iLoadedModules; ++i)
	{
		FEFAILEDRETURN(m_modules[i]->Unload());
	}
	return EFEReturnCode::Success;
}	  
uint32 FEApplication::Run()
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
		
		for (uint32 i = 0; i < m_iLoadedModules; ++i)
		{
			FEFAILEDRETURN(m_modules[i]->Update());
		}
	}

	return EFEReturnCode::Success;
}

int _tmain(int argc, _TCHAR* argv[])
{
	FEApplication app;

	FEFAILEDRETURN(app.Load());
	FEFAILEDRETURN(app.Run());
	FEFAILEDRETURN(app.Unload());

	return 0;
}