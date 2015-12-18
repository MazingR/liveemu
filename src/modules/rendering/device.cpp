#include <modulerenderer.hpp>

#include <windows.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>

namespace FeRendering
{
	FeRenderDevice FeModuleRendering::Device;

	void FeRenderDevice::Release()
	{
		if (ImmediateContext) ImmediateContext->ClearState();
		if (ImmediateContext) ImmediateContext->Release();
		if (SwapChain) SwapChain->Release();
		if (D3dDevice) D3dDevice->Release();
	}
	uint32 FeRenderDevice::Initialize(HWND hwnd)
	{
		HRESULT hr = S_OK;

		RECT rc;
		GetClientRect(hwnd, &rc);

		// Save up window resolution
		NativeResolution.w = rc.right - rc.left;
		NativeResolution.h = rc.bottom - rc.top;
		NativeResolution.x = 0;
		NativeResolution.y = 0;

		UINT createDeviceFlags = 0;
#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		D3D_DRIVER_TYPE driverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,
			D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE,
		};
		UINT numDriverTypes = ARRAYSIZE(driverTypes);

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		UINT numFeatureLevels = ARRAYSIZE(featureLevels);

		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = NativeResolution.w;
		sd.BufferDesc.Height = NativeResolution.h;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hwnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
		{
			DriverType = driverTypes[driverTypeIndex];
			hr = D3D11CreateDeviceAndSwapChain(NULL, DriverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
				D3D11_SDK_VERSION, &sd, &SwapChain, &D3dDevice, &FeatureLevel, &ImmediateContext);
			if (SUCCEEDED(hr))
				break;
		}
		if (FAILED(hr))
			return EFeReturnCode::Failed;

		return EFeReturnCode::Success;
	}

} // namespace FeRendering