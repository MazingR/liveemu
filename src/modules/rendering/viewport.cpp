#include <modulerenderer.hpp>

#include <d3dx11include.hpp>

#define D3DFAILEDRETURN(func) { HRESULT ___hr = (func); if (___hr!=S_OK) return ___hr; }

	void FeRenderViewport::Bind()  const
	{
		// Setup the viewport
		D3D11_VIEWPORT vp;
		ZeroMemory(&vp, sizeof(D3D11_VIEWPORT));

		vp.Width = (FLOAT)FeModuleRendering::GetDevice().GetNativeResolution().w;
		vp.Height = (FLOAT)FeModuleRendering::GetDevice().GetNativeResolution().h;
		vp.MaxDepth = 1.0f;

		FeModuleRendering::GetDevice().GetImmediateContext()->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);
		FeModuleRendering::GetDevice().GetImmediateContext()->RSSetViewports(1, &vp);
	}
	void FeRenderViewport::Clear()  const
	{
		const float ClearColor[4] = { 0.f, 0.f, 0.f, 1.0f };
		FeModuleRendering::GetDevice().GetImmediateContext()->ClearRenderTargetView(RenderTargetView, ClearColor);
		FeModuleRendering::GetDevice().GetImmediateContext()->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
	void FeRenderViewport::Unload()
	{
		SafeRelease(RenderTargetView);
		SafeRelease(DepthStencilView);
		SafeRelease(DepthStencil);
	}
	uint32 FeRenderViewport::CreateFromBackBuffer()
	{
		// Create a render target view
		ID3D11Texture2D* pBackBuffer = nullptr;

		D3DFAILEDRETURN(FeModuleRendering::GetDevice().GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer));
		D3DFAILEDRETURN(FeModuleRendering::GetDevice().GetD3DDevice()->CreateRenderTargetView(pBackBuffer, nullptr, &RenderTargetView));
		
		SafeRelease(pBackBuffer);

		// Create depth stencil texture
		D3D11_TEXTURE2D_DESC descDepth;
		ZeroMemory(&descDepth, sizeof(descDepth));

		descDepth.Width					= FeModuleRendering::GetDevice().GetNativeResolution().w;
		descDepth.Height				= FeModuleRendering::GetDevice().GetNativeResolution().h;
		descDepth.MipLevels				= 1;
		descDepth.ArraySize				= 1;
		descDepth.Format				= DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDepth.SampleDesc.Count		= 1;
		descDepth.SampleDesc.Quality	= 0;
		descDepth.Usage					= D3D11_USAGE_DEFAULT;
		descDepth.BindFlags				= D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags		= 0;
		descDepth.MiscFlags				= 0;

		HRESULT hr = FeModuleRendering::GetDevice().GetD3DDevice()->CreateTexture2D(&descDepth, nullptr, &DepthStencil);
		if (FAILED(hr))
			return FeEReturnCode::Failed;

		// Create the depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));

		descDSV.Format				= descDepth.Format;
		descDSV.ViewDimension		= D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice	= 0;

		Width = FeModuleRendering::GetDevice().GetNativeResolution().w;
		Height = FeModuleRendering::GetDevice().GetNativeResolution().h;

		hr = FeModuleRendering::GetDevice().GetD3DDevice()->CreateDepthStencilView(DepthStencil, &descDSV, &DepthStencilView);
		
		if (FAILED(hr))
			return FeEReturnCode::Failed;

		return FeEReturnCode::Success;
	}
