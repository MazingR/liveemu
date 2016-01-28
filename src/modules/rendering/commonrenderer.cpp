#include <commonrenderer.hpp>

#include <d3dx11include.hpp>
#include <ft2build.h>
#include <freetype/freetype.h>

void FeRenderTexture::Release()
{
 	SafeRelease(D3DResource);
	SafeRelease(D3DSRV);
}
void FeRenderFont::Release()
{
	//if (FtFontFace)
	//	delete FtFontFace;
}