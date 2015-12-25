#pragma once

#include <common/common.hpp>
#include <common/tarray.hpp>

using namespace FeCommon;

namespace FeRendering
{
	typedef uint32 FeRenderGeometryId;
	typedef uint32 FeRenderEffectId;
	typedef uint32 FeRenderTextureId;

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
		FeMatrix4	Matrix;
		static const FeMatrix4& IdentityMatrix();
	};
	struct FeRenderGeometryInstance
	{
		FeRenderGeometryId			Geometry;
		FeRenderEffectId			Effect;
		FeTArray<FeRenderTextureId>	Textures;
		FeGeometryTransform			Transform;

		FeRenderGeometryInstance()
		{
			Textures.Reserve(1);
		}
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
		static uint32 CreateStaticGeometry(FeEGemetryDataType::Type eType, FeRenderGeometryData* geometryData, FeRenderGeometryId* geometryId);
		static uint32 CreateGeometry(void* vertexBuffer, uint32 iVertexCount, void* indexBuffer, uint32 iIndexCount, FeRenderGeometryData* geometryData, FeRenderGeometryId* geometryId);
		static void ComputeAffineTransform(FeGeometryTransform& output, FeVector3 vTranslate, FeRotation vRotate, FeVector3 vScale);

		static void ReleaseGeometryData();
		static void ReleaseStaticGeometryData();
	private:
		static uint32 ComputeStaticGeometry();

		static FeCommon::FeTArray<FeRenderGeometryData> StaticGeometryData;
		static FeCommon::FeTArray<FeRenderGeometryData> GeometryData;
		static bool ComputedGeometry;
	};
}