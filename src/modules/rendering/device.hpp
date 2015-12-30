#pragma once

#include <commonrenderer.hpp>

class FeRenderDevice
{
public:
	uint32 Initialize(void* hwnd);
	void Release();
	ID3D11Device* GetD3DDevice() { FE_ASSERT(D3dDevice, "D3D Device is not initialized !");  return D3dDevice; }
	ID3D11DeviceContext* GetImmediateContext() { return ImmediateContext; }
	ID3D11DeviceContext* GetLoadingThreadContext() { return LoadingThreadContext; }
	IDXGISwapChain* GetSwapChain() { return SwapChain; }
	const FeRect& GetNativeResolution() { return NativeResolution; }

private:
	ID3D11Device*           D3dDevice;
	ID3D11DeviceContext*    ImmediateContext;
	ID3D11DeviceContext*    LoadingThreadContext;
	IDXGISwapChain*         SwapChain;
	FeRect					NativeResolution;
};