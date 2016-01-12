#pragma once

#include <common/common.hpp>
#include <common/tarray.hpp>
#include <common/application.hpp>
#include <common/filesystem.hpp>

struct FeRenderGeometryInstance;

struct FeModuleUiInit : public FeModuleInit
{
public:
};

class FeModuleUi : public FeModule
{
public:
	virtual uint32 Load(const FeModuleInit*) override;
	virtual uint32 Unload() override;
	virtual uint32 Update(const FeDt& fDt) override;

private:
	FeTArray<FeRenderGeometryInstance> GeometryInstances;
};

#include <common/serializable.hpp>
#include <common/maths.hpp>

class FeTestObjectBase : public FeSerializable
{
public:
	#define FeTestObjectBase_Properties(_d)		\
		_d(int,				Count)				\
		_d(float,			FValue)				\
		_d(FeTArray<int>,	Values)				\
		_d(FeTransform,		Transform)			\

	FE_DECLARE_CLASS_BODY(FeTestObjectBase_Properties, FeTestObjectBase, FeSerializable)
};
FE_DECLARE_CLASS_BOTTOM(FeTestObjectBase)


class FeTestObjectChild : public FeTestObjectBase
{
public:
	#define FeTestObjectChild_Properties(_d)\
	_d(FePath,	File)						\

	FE_DECLARE_CLASS_BODY(FeTestObjectChild_Properties, FeTestObjectChild, FeTestObjectBase)
};
FE_DECLARE_CLASS_BOTTOM(FeTestObjectChild)

class FeTestObjectA : public FeSerializable
{
	#define FeTestObjectA_Properties(_d)		\
	_d(FeTArray<FeTPtr<FeTestObjectBase>>,Objs)	\

	FE_DECLARE_CLASS_BODY(FeTestObjectA_Properties, FeTestObjectA, FeSerializable)
};
FE_DECLARE_CLASS_BOTTOM(FeTestObjectA)
