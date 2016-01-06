
#include "fepch.h"
#include "ferenderer.h"

#if defined(FE_D3D11)
	#pragma message ("FE_D3D11 ==> Compiling FrontEnd Direct3D 11.x render api")
	#error d3d11 api is not implemented yet :-(
#elif defined(FE_D3D9x)
	#pragma message ("FE_D3D9x ==> Compiling FrontEnd Direct3D 9x render api")

	#include <d3d9.h>
	#include <d3dx9.h>
	#include <math.h>
	#include <dinput.h>

#elif defined(FE_OPENGL)
	#pragma message ("FE_OPENGL==> Compiling FrontEnd OpenGL render api")
	#error opengl api  is not implemented yet :-(
#else
	#error no render api defined !
#endif

feresult feRendererInit(feSRenderer* _pOut)
{
	_pOut->m_iSharedTargetHandle		= 0;
	_pOut->m_iSharedTargetHeight		= 480;
	_pOut->m_iSharedTargetWidth			= 640;
	
	_pOut->m_pD3dSharedTargetSurface	= 0;
	_pOut->m_pD3dSharedTargetTexture	= 0;
	_pOut->m_pD3dEmulatorBackbuffer		= 0;
	_pOut->m_pD3dDevice					= 0;

	return FE_RESULT_OK;
}
feresult feRendererInitSharedTarget(feSRenderer* _pRenderer)
{
#if defined(FE_D3D9x)
	IDirect3DDevice9* d3dDev = NULL;
	IDirect3DTexture9* pSharedTexture = NULL;
	HRESULT hr = S_OK;
#endif

	if (_pRenderer->m_iSharedTargetHandle == 0)
		return FE_RESULT_ERROR;

#if defined(FE_D3D9x)

	d3dDev = (IDirect3DDevice9 *)_pRenderer->m_pD3dDevice;

	// recover shared resource as D3D9 texture interface
	
	hr = IDirect3DDevice9_CreateTexture(d3dDev,
		_pRenderer->m_iSharedTargetWidth,
		_pRenderer->m_iSharedTargetHeight,
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&_pRenderer->m_pD3dSharedTargetTexture,
		&_pRenderer->m_iSharedTargetHandle);

	if (FAILED(hr))	return FE_RESULT_ERROR;

	pSharedTexture = (IDirect3DTexture9*)_pRenderer->m_pD3dSharedTargetTexture;
	
	hr = IDirect3DTexture9_GetSurfaceLevel(pSharedTexture, 0, &((IDirect3DSurface9*)_pRenderer->m_pD3dSharedTargetSurface));
	if (FAILED(hr))	return FE_RESULT_ERROR;

	// Retrieve pointer to Back Buffer texture
	hr = IDirect3DDevice9_GetRenderTarget(d3dDev, 0, &((IDirect3DSurface9*)_pRenderer->m_pD3dEmulatorBackbuffer));
	if (FAILED(hr))	return FE_RESULT_ERROR;

#else
#error no render api defined !
#endif

	return FE_RESULT_OK;

}
feresult feRendererUpdateSharedTarget(feSRenderer* _pRenderer)
{
#if defined(FE_D3D9x)
	HRESULT hr = S_OK;
	IDirect3DDevice9 * d3dDev = NULL;
	IDirect3DSurface9* pSharedSurface = NULL;
#endif

	if (_pRenderer->m_iSharedTargetHandle==0)
		return FE_RESULT_ERROR;

#if defined(FE_D3D9x)
	d3dDev = (IDirect3DDevice9 *)_pRenderer->m_pD3dDevice;
	pSharedSurface = (IDirect3DSurface9*)_pRenderer->m_pD3dSharedTargetSurface;

	if (pSharedSurface == 0)
		return FE_RESULT_ERROR;
	hr = IDirect3DDevice9_StretchRect(d3dDev, (IDirect3DSurface9*)_pRenderer->m_pD3dEmulatorBackbuffer, NULL, pSharedSurface, NULL, D3DTEXF_POINT);
	if (FAILED(hr))	return FE_RESULT_ERROR;

#else
#error no render api defined !
#endif

	return FE_RESULT_OK;
}