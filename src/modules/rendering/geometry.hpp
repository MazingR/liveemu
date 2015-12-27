#pragma once

#include <commonrenderer.hpp>

class FeGeometryHelper
{
public:
	static uint32 CreateVertexAndIndexBuffer(ID3D11Buffer** pVertexBuffer, ID3D11Buffer** pIndexBuffer, void* vertexData, uint32 vertexCount, void* indexData, uint32 indexCount);
	static uint32 CreateStaticGeometry(FeEGemetryDataType::Type eType, FeRenderGeometryData* geometryData, FeRenderGeometryId* geometryId);
	static uint32 CreateGeometry(void* vertexBuffer, uint32 iVertexCount, void* indexBuffer, uint32 iIndexCount, FeRenderGeometryData* geometryData, FeRenderGeometryId* geometryId);
	static void ComputeAffineTransform(FeGeometryTransform& output, FeVector3 vTranslate, FeRotation vRotate, FeVector3 vScale);

	static FeRenderGeometryId GetStaticGeometry(FeEGemetryDataType::Type eType);

	static void ReleaseGeometryData();
	static void ReleaseStaticGeometryData();
private:
	static uint32 ComputeStaticGeometry();

	static FeTArray<FeRenderGeometryData> StaticGeometryData;
	static FeTArray<FeRenderGeometryData> GeometryData;
	static bool ComputedGeometry;
};