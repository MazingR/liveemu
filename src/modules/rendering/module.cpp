#include <module.hpp>

#include <windows.h>
#include <d3dx11.h>
#include <d3dcompiler.h>
#include <xnamath.h>

#define D3DFAILEDRETURN(func) { HRESULT ___hr = (func); if (___hr!=S_OK) return ___hr; }

namespace FeRendering
{
	void FeRenderViewport::Bind()  const
	{
		// Setup the viewport
		D3D11_VIEWPORT vp;
		ZeroMemory(&vp, sizeof(D3D11_VIEWPORT));

		vp.Width = (FLOAT)FeModuleRendering::GetDevice().GetNativeResolution().w;
		vp.Height = (FLOAT)FeModuleRendering::GetDevice().GetNativeResolution().h;
		vp.MaxDepth = 1.0f;

		FeModuleRendering::GetDevice().GetImmediateContext()->OMSetRenderTargets(1, &RenderTargetView, NULL);
		FeModuleRendering::GetDevice().GetImmediateContext()->RSSetViewports(1, &vp);
	}
	void FeRenderViewport::Clear()  const
	{
		const float ClearColor[4] = { 0.f, 0.f, 0.f, 1.0f };
		FeModuleRendering::GetDevice().GetImmediateContext()->ClearRenderTargetView(RenderTargetView, ClearColor);
	}

	uint32 FeModuleRendering::Load(const ::FeCommon::FeModuleInit* initBase)
	{
		auto init = (FeModuleRenderingInit*)initBase;

		FE_FAILEDRETURN(Device.Initialize(init->WindowHandle));
		FeRenderEffect& newEffect = Effects.Add();
		FE_FAILEDRETURN(newEffect.CreateFromFile("../data/themes/common/shaders/Tutorial02.fx"));

		// DEBUG code 
		FeGeometryDataId geometryId;

		Geometries.Reserve(16);
		Geometries.SetZeroMemory();

		FeRenderGeometryData& geometryData = Geometries.Add();

		FeGeometryHelper::CreateStaticGeometry(FeEGemetryDataType::Quad, &geometryData, &geometryId);

		renderBatch.Viewport.CreateFromBackBuffer();
		FeRenderGeometryInstance& geomInstance = renderBatch.GeometryInstances.Add();

		geomInstance.Effect = 1;
		geomInstance.Geometry = geometryId;


		return EFeReturnCode::Success;
	}
	uint32 FeModuleRendering::Unload()
	{
		Device.Release();

		for (uint32 i = 0; i < Effects.GetSize(); ++i)
			Effects[i].Release();

		FeGeometryHelper::ReleaseGeometryData();
		FeGeometryHelper::ReleaseStaticGeometryData();

		return EFeReturnCode::Success;
	}
	uint32 FeModuleRendering::Update()
	{
		int iProcessedMsg = 0;
		int iMaxProcessedMsg = 3;

		MSG msg = { 0 };

		while (WM_QUIT != msg.message && iProcessedMsg++<iMaxProcessedMsg)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				RenderBatch(renderBatch);
			}
		}

		if (WM_QUIT == msg.message)
		{
			return EFeReturnCode::Canceled;
		}

		return EFeReturnCode::Success;
	}

	uint32 FeRenderViewport::CreateFromBackBuffer()
	{
		// Create a render target view
		ID3D11Texture2D* pBackBuffer = NULL;
		
		D3DFAILEDRETURN(FeModuleRendering::GetDevice().GetSwapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer));
		D3DFAILEDRETURN(FeModuleRendering::GetDevice().GetD3DDevice()->CreateRenderTargetView(pBackBuffer, NULL, &RenderTargetView));

		pBackBuffer->Release();

		return EFeReturnCode::Success;
	}
	void FeModuleRendering::RenderBatch(FeRenderBatch& batch)
	{
		// Clear the back buffer
		batch.Viewport.Clear();
		batch.Viewport.Bind();

		FeCommon::FeTArray<FeRenderGeometryInstance>& instances = batch.GeometryInstances;

		FeEffectId iLastEffectId = 0;
		FeGeometryDataId iLastGeometryId = 0;
		
		FeRenderGeometryData* pGeometryData = NULL;
		FeRenderEffect* pEffect = NULL;

		for (uint32 iInstanceIdx = 0; iInstanceIdx < instances.GetSize(); ++iInstanceIdx)
		{
			FeRenderGeometryInstance& geomInstance = instances[iInstanceIdx];

			if (geomInstance.Effect != iLastEffectId)
			{
				FE_ASSERT(Effects.GetSize() >= geomInstance.Effect, "Invalid effect Id !");
				pEffect = &Effects[geomInstance.Effect - 1];
				pEffect->Bind();

				iLastEffectId = geomInstance.Effect;
			}
			if (geomInstance.Geometry != iLastGeometryId)
			{
				FE_ASSERT(Geometries.GetSize() >= geomInstance.Geometry, "Invalid geometry Id !");
				pGeometryData = &Geometries[geomInstance.Geometry - 1];
				iLastGeometryId = geomInstance.Geometry;
				
				UINT offset = 0;
				Device.GetImmediateContext()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				Device.GetImmediateContext()->IASetVertexBuffers(0, 1, (ID3D11Buffer**)&pGeometryData->VertexBuffer, &pGeometryData->Stride, &offset);
				Device.GetImmediateContext()->IASetIndexBuffer((ID3D11Buffer*)pGeometryData->IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
			}

			Device.GetImmediateContext()->DrawIndexed(pGeometryData->IndexCount, 0, 0);

			// todo : Set properties
		}
		// Present the information rendered to the back buffer to the front buffer (the screen)
		Device.GetSwapChain()->Present(0, 0);
	}

} // namespace FeRendering
