#pragma once 

#include <common.hpp>

#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/Vec.h>
#include <gmtl/VecOps.h>
#include <gmtl/Generate.h>

typedef gmtl::Vec3f FeVector3;
typedef gmtl::Vec4f FeColor;
typedef gmtl::Matrix33f FeMatrix3;
typedef gmtl::EulerAngleXYZf FeRotation;
typedef gmtl::Vec4f FeVector4;
typedef gmtl::Matrix44f FeMatrix4;

const FeMatrix4 FeMatrix4Identity = gmtl::MAT_IDENTITY44F;
const FeMatrix3 FeMatrix3Identity = gmtl::MAT_IDENTITY33F;

struct FeTransform
{
	FeVector3	Translation;
	FeRotation	Rotation;
	FeVector3	Scale;

	void SetIdentity()
	{
		Translation = FeVector3(0.f, 0.f, 0.f);
		Rotation	= FeRotation::EulerAngle(0.f, 0.f, 0.f);
		Scale		= FeVector3(1.f, 1.f, 1.f);
	}
};
namespace FeMatrix
{
	inline FeMatrix4 FromTranslation(FeVector3 vec) { return gmtl::makeTrans< gmtl::Matrix44f >(vec); }
	inline FeMatrix4 FromRotation(FeRotation rot) { return gmtl::make< gmtl::Matrix44f >(rot); }
	inline FeMatrix4 FromScale(FeVector3 vec) { return gmtl::makeScale< gmtl::Matrix44f >(vec); }
}
namespace FeMath
{
	template <typename T> static inline T		Min(const T& a, const T& b)
	{
		return(a < b ? a : b);
	}
	template <typename T> static inline T		Max(const T& a, const T& b)
	{
		return(a > b ? a : b);
	}
	template <typename T> static inline T       Clamp(const T& a, const T& min, const T& max)
	{
		return Min<T>(max, Max<T>(min, a));
	}
	template <typename T> static inline T		Abs(const T& val)
	{
		return(val < 0 ? -val : val);
	}
	template <typename T> static inline T		Range(const T& min, const T& max, const T& val)
	{
		return (max > val ? (min < val ? val : min) : max);
	}
	template <typename T> static inline float	Ratio(const T& min, const T& max, const T& val)
	{
		if (val <= min) return 0.0f;
		if (val >= max) return 1.0f;

		return float(val - min) * (1.0f / float(max - min));
	}
};
