#pragma once

#include <common/common.hpp>

#pragma warning( disable : 4005)
#include <d3d11.h>

namespace FeRendering
{
	typedef uint32 FeShaderId;

	class FeRenderEffect
	{
	public:
		void Bind();
		void Release();
		uint32 CreateFromFile(const char* szFilePath);
	private:
		uint32 CompileShaderFromFile(const char* szFileName, const char* szEntryPoint, const char* szShaderModel, void** ppBlobOut);

		ID3D11VertexShader*     VertexShader;
		ID3D11PixelShader*      PixelShader;
		ID3D11InputLayout*      VertexLayout;
	};
}