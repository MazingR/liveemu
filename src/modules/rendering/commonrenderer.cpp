#include <commonrenderer.hpp>

#include <d3dx11include.hpp>
#include <ft2build.h>
#include <freetype/freetype.h>

void FeRenderTexture::Release()
{
 	SafeRelease(D3DResource);
	SafeRelease(D3DSRV);
}
ID3D11Resource* FeRenderTexture::GetD3DResource(uint32) const
{
	return this->D3DResource;
}
ID3D11ShaderResourceView* FeRenderTexture::GetD3DSRV(uint32) const
{
	return this->D3DSRV;
}

void FeRenderTargetTexture::Release()
{
	Texture.Release();
	// todo@mazingr : release stuff
}
ID3D11Resource* FeRenderTargetTexture::GetD3DResource(uint32) const
{
	return this->Texture.D3DResource;
}
ID3D11ShaderResourceView* FeRenderTargetTexture::GetD3DSRV(uint32) const
{
	return this->Texture.D3DSRV;
}
ID3D11Resource* FeRenderFont::GetD3DResource(uint32) const
{
	return this->Texture.D3DResource;
}
ID3D11ShaderResourceView* FeRenderFont::GetD3DSRV(uint32) const
{
	return this->Texture.D3DSRV;
}
void FeRenderFont::Release()
{
	Texture.Release();
	//SafeDelete(FtFontFace);
	// todo@mazingr : release stuff
}