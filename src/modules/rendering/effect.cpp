#include <effect.hpp>
#include <modulerenderer.hpp>

#include <d3dx11include.hpp>
#include <d3dcompiler.h>
#include <xnamath.h>

struct FeCBPerFrame
{
	XMMATRIX MatrixView;
	XMMATRIX MatrixProj;
	float Time;
	XMVECTOR Resolution;
};

struct FeCBPerObject
{
	XMMATRIX MatrixWorld;
	XMVECTOR UserData;
};

uint32 FeRenderEffect::CompileShaderFromFile(const char* szFileName, const char* szEntryPoint, const char* szShaderModel, void** ppBlobOut)
{
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	auto hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, (ID3DBlob**)ppBlobOut, &pErrorBlob, NULL);

	if (FAILED(hr))
	{
		FE_LOG("Shader compile error '%s' (HR=%d) : %s ", szFileName, hr, pErrorBlob ? (char*)pErrorBlob->GetBufferPointer() : "Unknown error");

		if (pErrorBlob) 
			pErrorBlob->Release();

		return FeEReturnCode::Rendering_CreateShaderFailed;
	}

	if (pErrorBlob)
		pErrorBlob->Release();

	return FeEReturnCode::Success;
}
void FeRenderEffect::Release()
{
	SafeRelease(VertexLayout);
	SafeRelease(VertexShader);
	SafeRelease(PixelShader);

	SafeRelease(CBPerFrame.Buffer);
	SafeRelease(CBPerObject.Buffer);
}
void FeRenderEffect::BeginFrame(const FeRenderCamera& camera, const FeRenderViewport& viewport, float fDt)
{
	ID3D11DeviceContext* pContext = FeModuleRendering::GetDevice().GetImmediateContext();

	XMVECTOR Eye	= XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f);
	XMVECTOR At		= XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR Up		= XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	FeCBPerFrame cbPerFrame;

	//cbPerFrame.MatrixProj = XMMatrixPerspectiveFovLH(XM_PIDIV4, ((float)viewport.Width / (float)viewport.Height), 0.01f, 100.0f);
	cbPerFrame.MatrixProj = XMMatrixOrthographicLH(1.0f, 1.0f, -1.0f, 1.0f);
	cbPerFrame.MatrixView = XMMatrixIdentity();// XMMatrixLookAtLH(Eye, At, Up);
	cbPerFrame.MatrixView._41 = -0.5f;
	cbPerFrame.MatrixView._42 = -0.5f;

	cbPerFrame.MatrixProj = XMMatrixTranspose(cbPerFrame.MatrixProj);
	cbPerFrame.MatrixView = XMMatrixTranspose(cbPerFrame.MatrixView);

	static float fTime = 0.0f;

	fTime += fDt;

	/*if (fTime > 1.0f)
		fTime = 0.0f;*/

	cbPerFrame.Time = fTime;
	cbPerFrame.Resolution = XMVectorSet((float)viewport.Width, (float)viewport.Height, 0.f, 0.f);

	FeMatrix4 matViewPorj;
	mult(matViewPorj, camera.MatrixProjection, camera.MatrixView);
		
	pContext->UpdateSubresource(CBPerFrame.Buffer, 0, NULL, &cbPerFrame, 0, 0);
}
void FeRenderEffect::EndFrame()
{

}
void FeRenderEffect::BindGeometryInstance(const FeRenderGeometryInstance geometryInstance, const FeModuleRenderResourcesHandler* resouresHandler)
{
	ID3D11DeviceContext* pContext = FeModuleRendering::GetDevice().GetImmediateContext();

	FeCBPerObject data;
	data.MatrixWorld = XMMatrixTranspose(geometryInstance.Transform.Matrix.getData());

	if (Type == FeERenderEffectType::Font)
	{
		const float* pUserData = geometryInstance.UserData.getData();
		data.UserData = XMVectorSet(pUserData[0], pUserData[1], pUserData[2], pUserData[3]);
	}
	pContext->UpdateSubresource(CBPerObject.Buffer, 0, NULL, &data, 0, 0);
	
	// todo: bind other constants
}
void FeRenderEffect::Bind()
{
	ID3D11DeviceContext* pContext = FeModuleRendering::GetDevice().GetImmediateContext();

	pContext->IASetInputLayout((ID3D11InputLayout*)VertexLayout);
	pContext->VSSetShader(VertexShader, NULL, 0);
	pContext->PSSetShader(PixelShader, NULL, 0);

	pContext->VSSetConstantBuffers(0, 1, &CBPerFrame.Buffer);
	pContext->VSSetConstantBuffers(1, 1, &CBPerObject.Buffer);

	pContext->PSSetConstantBuffers(0, 1, &CBPerFrame.Buffer);
	pContext->PSSetConstantBuffers(1, 1, &CBPerObject.Buffer);

	pContext->PSSetSamplers(0, 1, &Samplers[0].State);

	if (UseAlphaBlending)
		pContext->OMSetBlendState(BlendState, 0, 0xffffffff);
	else
		pContext->OMSetBlendState(NULL, 0, 0xffffffff);
}
uint32 FeRenderEffect::CreateFromFile(const char* szFilePath)
{
	// Compile the vertex shader
	HRESULT hr = S_OK;
	ID3DBlob* pVSBlob = NULL;
	ID3D11Device* pD3DDevice = FeModuleRendering::GetDevice().GetD3DDevice();

	char szFullPath[1024];
	sprintf_s(szFullPath, "%s%s", FeFileTools::GetRootDir().Value, szFilePath);
	FE_FAILEDRETURN( CompileShaderFromFile(szFullPath, "VS", "vs_5_0", (void**)&pVSBlob) );

	// Create the vertex shader
	hr = pD3DDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, (ID3D11VertexShader**)&VertexShader);
		
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return FeEReturnCode::Rendering_CreateShaderFailed;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		0, 3*4,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = pD3DDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), (ID3D11InputLayout**)&VertexLayout);
	pVSBlob->Release();
	if (FAILED(hr)) return FeEReturnCode::Rendering_CreateShaderFailed;

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile(szFullPath, "PS", "ps_5_0", (void**)&pPSBlob);

	if (FAILED(hr) || !pPSBlob)
	{
		MessageBox(NULL, "The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", "Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = pD3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, (ID3D11PixelShader**)&PixelShader);
	pPSBlob->Release();
	if (FAILED(hr)) return FeEReturnCode::Rendering_CreateShaderFailed;

	// Create the constant buffers
	{
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(FeCBPerFrame);
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = 0;
		hr = pD3DDevice->CreateBuffer(&desc, NULL, &CBPerFrame.Buffer);
		if (FAILED(hr))
			return FeEReturnCode::Failed;

		desc.ByteWidth = sizeof(FeCBPerObject);
		hr = pD3DDevice->CreateBuffer(&desc, NULL, &CBPerObject.Buffer);

		if (FAILED(hr))
			return FeEReturnCode::Failed;
	}
	// Create the sample state
	{
		Samplers.Reserve(1);
		FeRenderSampler& sampler = Samplers.Add();

		D3D11_SAMPLER_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		desc.MinLOD = 0;
		desc.MaxLOD = D3D11_FLOAT32_MAX;
		hr = pD3DDevice->CreateSamplerState(&desc, &sampler.State);

		if (FAILED(hr))
			return FeEReturnCode::Failed;
	}
	// Create blend state
	{
		D3D11_BLEND_DESC desc;
		ZeroMemory(&desc, sizeof(desc));

		D3D11_RENDER_TARGET_BLEND_DESC& targetDesc = desc.RenderTarget[0];
		targetDesc.BlendEnable = true;
		targetDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		targetDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		targetDesc.BlendOp = D3D11_BLEND_OP_ADD;
		targetDesc.SrcBlendAlpha = D3D11_BLEND_ZERO;
		targetDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
		targetDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		targetDesc.RenderTargetWriteMask = 0x0F;

		hr = pD3DDevice->CreateBlendState(&desc, &BlendState);
		if (FAILED(hr))
			return FeEReturnCode::Failed;
	}

	return FeEReturnCode::Success;
}
