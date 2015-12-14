#pragma once

#include <common/common.hpp>


#pragma warning( disable : 4005)
#include <d3d11.h>

namespace FeRendering
{
	class FeRenderDevice
	{
	public:
		uint32 Initialize(HWND hwnd);
		void Release();
		ID3D11Device* GetD3DDevice() { FE_ASSERT(D3dDevice, "D3D Device is not initialized !");  return D3dDevice; }
		ID3D11DeviceContext* GetImmediateContext() { return ImmediateContext; }
		IDXGISwapChain* GetSwapChain() { return SwapChain; }
		const FeRect& GetNativeResolution() { return NativeResolution; }

	private:
		D3D_DRIVER_TYPE         DriverType = D3D_DRIVER_TYPE_NULL;
		D3D_FEATURE_LEVEL       FeatureLevel = D3D_FEATURE_LEVEL_11_0;

		ID3D11Device*           D3dDevice;
		ID3D11DeviceContext*    ImmediateContext;
		IDXGISwapChain*         SwapChain;
		FeRect					NativeResolution;
	};
}