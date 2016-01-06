#pragma once

HEADER_TOP

typedef struct feSRenderer
{
	void*	m_iSharedTargetHandle;
	int		m_iSharedTargetWidth;
	int		m_iSharedTargetHeight;

	void*	m_pD3dSharedTargetSurface;
	void*	m_pD3dSharedTargetTexture;
	void*	m_pD3dEmulatorBackbuffer;
	void*	m_pD3dDevice;

} feSRenderer;

feresult feRendererInit(feSRenderer* _pRenderer);
feresult feRendererInitSharedTarget(feSRenderer* _pRenderer);
feresult feRendererUpdateSharedTarget(feSRenderer* _pRenderer);

HEADER_BOTTOM