#include <module.hpp>

#include <windows.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>

#define D3DFAILEDRETURN(func) { HRESULT ___hr = (func); if (___hr!=S_OK) return ___hr; }

namespace FeRendering
{
	void FeRenderViewport::Bind()  const
	{
		// Setup the viewport
		D3D11_VIEWPORT vp;
		ZeroMemory(&vp, sizeof(D3D11_VIEWPORT));

		vp.Width = (FLOAT)FeModuleRendering::GetDevice().GetNativeResolution().w;
		vp.Height = (FLOAT)FeModuleRendering::GetDevice().GetNativeResolution().h;
		vp.MaxDepth = 1.0f;

		FeModuleRendering::GetDevice().GetImmediateContext()->OMSetRenderTargets(1, &RenderTargetView, NULL);
		FeModuleRendering::GetDevice().GetImmediateContext()->RSSetViewports(1, &vp);
	}
	void FeRenderViewport::Clear()  const
	{
		const float ClearColor[4] = { 0.f, 0.f, 0.f, 1.0f };
		FeModuleRendering::GetDevice().GetImmediateContext()->ClearRenderTargetView(RenderTargetView, ClearColor);
	}

	uint32 FeModuleRendering::Load(const ::FeCommon::FeModuleInit* initBase)
	{
		auto init = (FeModuleRenderingInit*)initBase;

		FE_FAILEDRETURN(Device.Initialize(init->WindowHandle));
		
		FeRenderEffect& newEffect = Effects.Add();
		FE_FAILEDRETURN(newEffect.CreateFromFile("../data/themes/common/shaders/Tutorial02.fx"));


		// DEBUG code 
		FeGeometryDataId geometryId;

		Geometries.Reserve(16);
		Geometries.SetZeroMemory();

		FeRenderGeometryData& geometryData = Geometries.Add();

		FeGeometryHelper::CreateStaticGeometry(FeEGemetryDataType::Quad, &geometryData, &geometryId);

		renderBatch.Viewport.CreateFromBackBuffer();
		FeRenderGeometryInstance& geomInstance = renderBatch.GeometryInstances.Add();

		geomInstance.Effect = 1;
		geomInstance.Geometry = geometryId;


		return EFeReturnCode::Success;
	}
	uint32 FeModuleRendering::Unload()
	{
		Device.Release();

		for (uint32 i = 0; i < Effects.GetSize(); ++i)
			Effects[i].Release();

		FeGeometryHelper::ReleaseGeometryData();
		FeGeometryHelper::ReleaseStaticGeometryData();

		return EFeReturnCode::Success;
	}
	uint32 FeModuleRendering::Update()
	{
		int iProcessedMsg = 0;
		int iMaxProcessedMsg = 3;

		MSG msg = { 0 };

		while (WM_QUIT != msg.message && iProcessedMsg++<iMaxProcessedMsg)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				RenderBatch(renderBatch);
			}
		}

		if (WM_QUIT == msg.message)
		{
			return EFeReturnCode::Canceled;
		}

		return EFeReturnCode::Success;
	}

	uint32 FeRenderViewport::CreateFromBackBuffer()
	{
		// Create a render target view
		ID3D11Texture2D* pBackBuffer = NULL;
		
		D3DFAILEDRETURN(FeModuleRendering::GetDevice().GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer));
		D3DFAILEDRETURN(FeModuleRendering::GetDevice().GetD3DDevice()->CreateRenderTargetView(pBackBuffer, NULL, &RenderTargetView));

		pBackBuffer->Release();

		return EFeReturnCode::Success;
	}
	void FeModuleRendering::RenderBatch(FeRenderBatch& batch)
	{
		// Clear the back buffer
		batch.Viewport.Clear();
		batch.Viewport.Bind();

		FeCommon::FeTArray<FeRenderGeometryInstance>& instances = batch.GeometryInstances;

		FeEffectId iLastEffectId = 0;
		FeGeometryDataId iLastGeometryId = 0;
		
		FeRenderGeometryData* pGeometryData = NULL;
		FeRenderEffect* pEffect = NULL;

		for (uint32 iInstanceIdx = 0; iInstanceIdx < instances.GetSize(); ++iInstanceIdx)
		{
			FeRenderGeometryInstance& geomInstance = instances[iInstanceIdx];

			if (geomInstance.Effect != iLastEffectId)
			{
				FE_ASSERT(Effects.GetSize() >= geomInstance.Effect, "Invalid effect Id !");
				pEffect = &Effects[geomInstance.Effect - 1];
				pEffect->Bind();

				iLastEffectId = geomInstance.Effect;
			}
			if (geomInstance.Geometry != iLastGeometryId)
			{
				FE_ASSERT(Geometries.GetSize() >= geomInstance.Geometry, "Invalid geometry Id !");
				pGeometryData = &Geometries[geomInstance.Geometry - 1];
				iLastGeometryId = geomInstance.Geometry;
				
				UINT offset = 0;
				Device.GetImmediateContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				Device.GetImmediateContext()->IASetVertexBuffers(0, 1, (ID3D11Buffer**)&pGeometryData->VertexBuffer, &pGeometryData->Stride, &offset);
				Device.GetImmediateContext()->IASetIndexBuffer((ID3D11Buffer*)pGeometryData->IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
			}

			Device.GetImmediateContext()->DrawIndexed(pGeometryData->IndexCount, 0, 0);

			// todo : Set properties
		}
		// Present the information rendered to the back buffer to the front buffer (the screen)
		Device.GetSwapChain()->Present(0, 0);
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