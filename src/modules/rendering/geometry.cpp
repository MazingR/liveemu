#include <modulerenderer.hpp>

#include <windows.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>

#define D3DFAILEDRETURN(func) { HRESULT ___hr = (func); if (___hr!=S_OK) return ___hr; }

namespace FeRendering
{
	struct Vertex_T0
	{
		XMFLOAT3 Position;
		XMFLOAT2 Texcoord0;
	};

	FeCommon::FeTArray<FeRenderGeometryData> FeGeometryHelper::StaticGeometryData;
	FeCommon::FeTArray<FeRenderGeometryData> FeGeometryHelper::GeometryData;
	bool FeGeometryHelper::ComputedGeometry = false;

	void FeRenderGeometryData::Release()
	{
		if (VertexBuffer) ((ID3D11Buffer*)VertexBuffer)->Release();
		if (VertexBuffer) ((ID3D11Buffer*)IndexBuffer)->Release();
	}
	uint32 FeGeometryHelper::CreateVertexAndIndexBuffer(void** pVertexBuffer, void** pIndexBuffer, void* vertexData, uint32 vertexCount, void* indexData, uint32 indexCount)
	{
		D3D11_BUFFER_DESC bd;
		D3D11_SUBRESOURCE_DATA InitData;
		{
			ZeroMemory(&bd, sizeof(bd));
			ZeroMemory(&InitData, sizeof(InitData));

			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(Vertex_T0) * vertexCount;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			InitData.pSysMem = vertexData;

			D3DFAILEDRETURN(FeModuleRendering::GetDevice().GetD3DDevice()->CreateBuffer(&bd, &InitData, (ID3D11Buffer**)pVertexBuffer));
		}
		{
			ZeroMemory(&bd, sizeof(bd));
			ZeroMemory(&InitData, sizeof(InitData));

			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(uint16) * indexCount;
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			InitData.pSysMem = indexData;

			D3DFAILEDRETURN(FeModuleRendering::GetDevice().GetD3DDevice()->CreateBuffer(&bd, &InitData, (ID3D11Buffer**)pIndexBuffer));
		}
		return EFeReturnCode::Success;
	}
	uint32 FeGeometryHelper::ComputeStaticGeometry()
	{
		StaticGeometryData.Resize(FeEGemetryDataType::Count);	
		StaticGeometryData.SetZeroMemory();
		
		{
			// Create vertex buffer
			Vertex_T0 vertexData[] =
			{
				{ XMFLOAT3(0.0f, 0.0f, 0.f)		, XMFLOAT2(0.f, 0.f) },
				{ XMFLOAT3(1.0f, 0.0f, 0.f)		, XMFLOAT2(1.f, 0.f) },
				{ XMFLOAT3(1.0f, -1.0f, 0.f)	, XMFLOAT2(1.f, 1.f) },
				{ XMFLOAT3(0.0f, -1.0f, 0.f)	, XMFLOAT2(0.f, 1.f) },
			};
			uint16 indexData[] =
			{
				0, 1, 2,
				0, 2, 3
			};
			StaticGeometryData[FeEGemetryDataType::Quad].Stride = sizeof(Vertex_T0);
			StaticGeometryData[FeEGemetryDataType::Quad].IndexCount = 6;
			
			CreateVertexAndIndexBuffer(
				&StaticGeometryData[FeEGemetryDataType::Quad].VertexBuffer,
				&StaticGeometryData[FeEGemetryDataType::Quad].IndexBuffer, vertexData, 4, indexData, 6);
		}

		return EFeReturnCode::Success;
	}
	uint32 FeGeometryHelper::CreateStaticGeometry(FeEGemetryDataType::Type eType, FeRenderGeometryData* geometryData, FeGeometryDataId* geometryId)
	{
		if (!ComputedGeometry)
		{
			FE_FAILEDRETURN(ComputeStaticGeometry());
			ComputedGeometry = true;
		}
		*geometryData = StaticGeometryData[eType];
		*geometryId = (FeGeometryDataId)eType + 1;

		return EFeReturnCode::Success;
	}
	uint32 FeGeometryHelper::CreateGeometry(void* vertexBuffer, uint32 iVertexCount, void* indexBuffer, uint32 iIndexCount, FeRenderGeometryData* geometryData, FeGeometryDataId* geometryId)
	{
		FeRenderGeometryData& newGeometryData = GeometryData.Add();
		*geometryId = GeometryData.GetSize();
		return EFeReturnCode::Success;
	}
	
	void FeGeometryHelper::ReleaseGeometryData()
	{
		for (uint32 i = 0; i < GeometryData.GetSize(); ++i)
			GeometryData[i].Release();
	}
	void FeGeometryHelper::ReleaseStaticGeometryData()
	{
		for (uint32 i = 0; i < StaticGeometryData.GetSize(); ++i)
			StaticGeometryData[i].Release();
	}

} // namespace FeRendering
