#pragma once

#include <common/common.hpp>

namespace FeRendering
{
	typedef uint32 FeGeometryDataId;
	typedef uint32 FeEffectId;

	struct FeRenderGeometryData
	{
		void*	VertexBuffer;
		void*	IndexBuffer;
		uint32	Stride;
		uint32	IndexCount;

		void Release();
	};
	
	struct FeGeometryTransform
	{

	};
	struct FeRenderGeometryInstance
	{
		FeGeometryDataId	Geometry;
		FeEffectId			Effect;
		FeGeometryTransform	Transform;
	};
	namespace FeEGemetryDataType
	{
		enum Type
		{
			Quad,
			Count
		};
	};
	class FeGeometryHelper
	{
	public:
		static uint32 CreateVertexAndIndexBuffer(void** pVertexBuffer, void** pIndexBuffer, void* vertexData, uint32 vertexCount, void* indexData, uint32 indexCount);
		static uint32 CreateStaticGeometry(FeEGemetryDataType::Type eType, FeRenderGeometryData* geometryData, FeGeometryDataId* geometryId);
		static uint32 CreateGeometry(void* vertexBuffer, uint32 iVertexCount, void* indexBuffer, uint32 iIndexCount, FeRenderGeometryData* geometryData, FeGeometryDataId* geometryId);

		static void ReleaseGeometryData();
		static void ReleaseStaticGeometryData();
	private:
		static uint32 ComputeStaticGeometry();

		static FeCommon::FeTArray<FeRenderGeometryData> StaticGeometryData;
		static FeCommon::FeTArray<FeRenderGeometryData> GeometryData;
		static bool ComputedGeometry;
	};
}