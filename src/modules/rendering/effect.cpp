#include <module.hpp>
#include <device.hpp>

#include <windows.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>

namespace FeRendering
{
	uint32 FeRenderEffect::CompileShaderFromFile(const char* szFileName, const char* szEntryPoint, const char* szShaderModel, void** ppBlobOut)
	{
		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
		dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

		ID3DBlob* pErrorBlob;
		auto hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
			dwShaderFlags, 0, NULL, (ID3DBlob**)ppBlobOut, &pErrorBlob, NULL);

		if (FAILED(hr))
		{
			FE_LOG("Shader compile error '%s' : %d \n %s ", szFileName, hr, pErrorBlob ? (char*)pErrorBlob->GetBufferPointer() : "");
			if (pErrorBlob) 
				pErrorBlob->Release();

			return EFeReturnCode::Rendering_CreateShaderFailed;
		}

		if (pErrorBlob)
			pErrorBlob->Release();

		return EFeReturnCode::Success;
	}
	void FeRenderEffect::Release()
	{
		if (VertexLayout) VertexLayout->Release();
		if (VertexShader) VertexShader->Release();
		if (PixelShader) PixelShader->Release();
	}
	void FeRenderEffect::Bind()
	{
		FeModuleRendering::GetDevice().GetImmediateContext()->IASetInputLayout((ID3D11InputLayout*)VertexLayout);
		FeModuleRendering::GetDevice().GetImmediateContext()->VSSetShader(VertexShader, NULL, 0);
		FeModuleRendering::GetDevice().GetImmediateContext()->PSSetShader(PixelShader, NULL, 0);

	}
	uint32 FeRenderEffect::CreateFromFile(const char* szFilePath)
	{
		// Compile the vertex shader
		HRESULT hr = S_OK;
		ID3DBlob* pVSBlob = NULL;
		ID3D11Device* pD3DDevice = FeModuleRendering::GetDevice().GetD3DDevice();

		FE_FAILEDRETURN( CompileShaderFromFile(szFilePath, "VS", "vs_4_0", (void**)&pVSBlob) );

		// Create the vertex shader
		hr = pD3DDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, (ID3D11VertexShader**)&VertexShader);
		
		if (FAILED(hr))
		{
			pVSBlob->Release();
			return EFeReturnCode::Rendering_CreateShaderFailed;
		}

		// Define the input layout
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,			0, 4*3,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE(layout);

		// Create the input layout
		hr = pD3DDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), (ID3D11InputLayout**)&VertexLayout);
		pVSBlob->Release();
		if (FAILED(hr)) return EFeReturnCode::Rendering_CreateShaderFailed;

		// Compile the pixel shader
		ID3DBlob* pPSBlob = NULL;
		hr = CompileShaderFromFile(szFilePath, "PS", "ps_4_0", (void**)&pPSBlob);

		if (FAILED(hr))
		{
			MessageBox(NULL, "The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", "Error", MB_OK);
			return hr;
		}

		// Create the pixel shader
		hr = pD3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, (ID3D11PixelShader**)&PixelShader);
		pPSBlob->Release();
		if (FAILED(hr)) return EFeReturnCode::Rendering_CreateShaderFailed;

		return EFeReturnCode::Success;
	}
} // namespace FeRendering


/*
class FeModuleRenderer : public FeModule
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
uint32 FeModuleRenderer::Unload()
{
SDL_DestroyTexture(tex);
SDL_DestroyRenderer(ren);
SDL_DestroyWindow(win);
SDL_Quit();

return EFeReturnCode::Success;
}
uint32 FeModuleRenderer::Load()
{
if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
{
std::cerr << "SDL_Init error: " << SDL_GetError() << std::endl;
return EFeReturnCode::Failed;
}
FE_LOG("Resource path is: %s", getResourcePath());

win = SDL_CreateWindow("Hello World!", 100, 100, 640, 480, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE); //SDL_WINDOW_FULLSCREEN_DESKTOP
if (win == nullptr)
{
FE_LOG("SDL_CreateWindow Error: %s",SDL_GetError());
SDL_Quit();
return EFeReturnCode::Failed;
}


ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
if (ren == nullptr){
SDL_DestroyWindow(win);
FE_LOG("SDL_CreateRenderer Error: %s", SDL_GetError());
SDL_Quit();
return EFeReturnCode::Failed;
}

std::string imagePath = getResourcePath("Lesson1") + "lena512.bmp";
SDL_Surface *bmp = SDL_LoadBMP(imagePath.c_str());
if (bmp == nullptr){
SDL_DestroyRenderer(ren);
SDL_DestroyWindow(win);
FE_LOG("SDL_LoadBMP Error: %s", SDL_GetError());
SDL_Quit();
return EFeReturnCode::Failed;
}

tex = SDL_CreateTextureFromSurface(ren, bmp);
SDL_FreeSurface(bmp);

if (tex == nullptr){
SDL_DestroyRenderer(ren);
SDL_DestroyWindow(win);
FE_LOG("SDL_CreateTextureFromSurface Error: %s", SDL_GetError());
SDL_Quit();
return EFeReturnCode::Failed;
}

return EFeReturnCode::Success;
}
uint32 FeModuleRenderer::Update()
{
//First clear the renderer
SDL_RenderClear(ren);
//Draw the texture
SDL_RenderCopy(ren, tex, NULL, NULL);
//Update the screen
SDL_RenderPresent(ren);

//Take a quick break after all that hard work
//SDL_Delay(1000);

return EFeReturnCode::Success;
}
*/