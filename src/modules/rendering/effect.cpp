#include <effect.hpp>
#include <modulerenderer.hpp>

#include <d3dx11include.hpp>
#include <d3dcompiler.h>
#include <xnamath.h>

struct FeCBPerFrame
{
	XMMATRIX MatrixView;
	XMMATRIX MatrixProj;
};

struct FeCBPerObject
{
	XMMATRIX MatrixWorld;
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
		FE_LOG("Shader compile error '%s' : %d \n %s ", szFileName, hr, pErrorBlob ? (char*)pErrorBlob->GetBufferPointer() : "");
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
void FeRenderEffect::BeginFrame(const FeRenderCamera& camera, const FeRenderViewport& viewport)
{
	ID3D11DeviceContext* pContext = FeModuleRendering::GetDevice().GetImmediateContext();

	XMVECTOR Eye = XMVectorSet(0.0f, 0.0f, -8.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	FeCBPerFrame cbPerFrame;

	cbPerFrame.MatrixProj = XMMatrixPerspectiveFovLH(XM_PIDIV4, ((float)viewport.Width / (float)viewport.Height), 0.01f, 100.0f);
	cbPerFrame.MatrixView = XMMatrixLookAtLH(Eye, At, Up);

	cbPerFrame.MatrixProj = XMMatrixTranspose(cbPerFrame.MatrixProj);
	cbPerFrame.MatrixView = XMMatrixTranspose(cbPerFrame.MatrixView);

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
	pContext->PSSetSamplers(0, 1, &Samplers[0].State);
}
uint32 FeRenderEffect::CreateFromFile(const char* szFilePath)
{
	// Compile the vertex shader
	HRESULT hr = S_OK;
	ID3DBlob* pVSBlob = NULL;
	ID3D11Device* pD3DDevice = FeModuleRendering::GetDevice().GetD3DDevice();

	FE_FAILEDRETURN( CompileShaderFromFile(szFilePath, "VS", "vs_4_0", (void**)&pVSBlob) );

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
	hr = CompileShaderFromFile(szFilePath, "PS", "ps_4_0", (void**)&pPSBlob);

	if (FAILED(hr))
	{
		MessageBox(NULL, "The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", "Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = pD3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, (ID3D11PixelShader**)&PixelShader);
	pPSBlob->Release();
	if (FAILED(hr)) return FeEReturnCode::Rendering_CreateShaderFailed;
		
	// Create the constant buffers
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	ZeroMemory(&CBPerFrame, sizeof(FeCBPerFrame));
	ZeroMemory(&CBPerObject, sizeof(FeCBPerObject));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(FeCBPerFrame);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = pD3DDevice->CreateBuffer(&bd, NULL, &CBPerFrame.Buffer);
	if (FAILED(hr))
		return FeEReturnCode::Failed;

	bd.ByteWidth = sizeof(FeCBPerObject);
	hr = pD3DDevice->CreateBuffer(&bd, NULL, &CBPerObject.Buffer);
	if (FAILED(hr))
		return FeEReturnCode::Failed;

	// Create the sample state
	Samplers.Reserve(1);
	FeRenderSampler& sampler = Samplers.Add();

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = pD3DDevice->CreateSamplerState(&sampDesc, &sampler.State);

	if (FAILED(hr))
		return FeEReturnCode::Failed;


	return FeEReturnCode::Success;
}
