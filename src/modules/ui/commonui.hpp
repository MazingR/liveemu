#pragma once

#include <rendering/effect.hpp>
#include <common/serializable.hpp>
#include <common/maths.hpp>

struct FeETargetPropertyType
{
	enum Type
	{
		Text,
		Font,
		Image,
		Other,
	};
};

struct FeEUiElementState
{
	enum Type
	{
		Visible = 1 << 0,
		Enabled = 1 << 1,
		Collapsed = 1 << 2,
		Focused = 1 << 3,
		Selected = 1 << 4,
	};
};

#define FeEUiBindingType_Values(_d)		\
		_d(FeEUiBindingType, Variable)	\
		_d(FeEUiBindingType, Database)	\
		_d(FeEUiBindingType, Static)	\

FE_DECLARE_ENUM(FeEUiBindingType, FeEUiBindingType_Values)

class FeUiBinding : public FeSerializable
{
public:
#define FeUiBinding_Properties(_d)				\
		_d(FeNTArray<FeString>,		Path)		\
		_d(FeString,				Property)	\
		_d(uint32,					Index)		\
		_d(FeString,				Value)		\
		_d(FeEUiBindingType::Type,	Type)		\

	FE_DECLARE_CLASS_BODY(FeUiBinding_Properties, FeUiBinding, FeSerializable)
};
FE_DECLARE_CLASS_BOTTOM(FeUiBinding)

class FeUiDataBinding : public FeSerializable
{
public:

#define FeUiDataBinding_Properties(_d)		\
		_d(FeUiBinding,	Source)				\
		_d(FeUiBinding,	Target)				\

	FE_DECLARE_CLASS_BODY(FeUiDataBinding_Properties, FeUiDataBinding, FeSerializable)
};
FE_DECLARE_CLASS_BOTTOM(FeUiDataBinding)
