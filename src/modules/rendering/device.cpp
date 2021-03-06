#include <device.hpp>
#include <modulerenderer.hpp>

#include <d3dx11include.hpp>

FeRenderDevice FeModuleRendering::Device;

void FeRenderDevice::Release()
{
	if (ImmediateContext) ImmediateContext->ClearState();
	if (ImmediateContext) ImmediateContext->Release();

	if (LoadingThreadContext) LoadingThreadContext->ClearState();
	if (LoadingThreadContext) LoadingThreadContext->Release();
	
	if (SwapChain) SwapChain->Release();
	if (D3dDevice) D3dDevice->Release();
}
uint32 FeRenderDevice::Initialize(void* hwnd)
{
	HRESULT hr = S_OK;
	D3D_DRIVER_TYPE         DriverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL       FeatureLevel = D3D_FEATURE_LEVEL_11_0;

	RECT rc;
	GetClientRect((HWND)hwnd, &rc);

	// Save up window resolution
	NativeResolution.w = rc.right - rc.left;
	NativeResolution.h = rc.bottom - rc.top;
	NativeResolution.x = 0;
	NativeResolution.y = 0;

	UINT createDeviceFlags = 0;
#ifdef DEBUG
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
	sd.OutputWindow = (HWND)hwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		DriverType = driverTypes[driverTypeIndex];
		
		hr = D3D11CreateDeviceAndSwapChain(nullptr, DriverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &SwapChain, &D3dDevice, &FeatureLevel, &ImmediateContext);
		
		if (SUCCEEDED(hr))
			break;
	}
	
	if (FAILED(hr))
		return FeEReturnCode::Failed;

	// create deferred contexts
	D3dDevice->CreateDeferredContext(0, &LoadingThreadContext);

	return FeEReturnCode::Success;
}
