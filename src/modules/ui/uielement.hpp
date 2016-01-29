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


#define FeEUiElementState_Values(_d)		\
		_d(FeEUiElementState, Visible)		\
		_d(FeEUiElementState, Collapsed)	\
		_d(FeEUiElementState, Selected)		\

FE_DECLARE_ENUM(FeEUiElementState, FeEUiElementState_Values)

#define FeEUiBindingType_Values(_d)				\
		_d(FeEUiBindingType, Source_Variable)	\
		_d(FeEUiBindingType, Source_List)		\
		_d(FeEUiBindingType, Source_Static)		\

FE_DECLARE_ENUM(FeEUiBindingType, FeEUiBindingType_Values)

class FeUiBinding : public FeSerializable
{
public:
#define FeUiBinding_Properties(_d)				\
		_d(FeTArray<FeString>,		Path)		\
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

class FeUiElement : public FeSerializable
{
public:
	bool HasState(FeEUiElementState::Type state);
	bool IsVisible();
	bool IsSelected();

	#define FeUiElement_Properties(_d)						\
		_d(FeString,						Name)			\
		_d(FeTransform,						Transform)		\
		_d(FeEUiElementState::Type,			State)			\
		_d(FeTArray<FeTPtr<FeUiElement>>,	Children)		\
		_d(FeString,						Effect)			\
		_d(FeString,						FontEffect)		\
		_d(FeString,						Font)			\
		_d(FeTArray<FeUiDataBinding>,		Bindings)		\

	FE_DECLARE_CLASS_BODY(FeUiElement_Properties, FeUiElement, FeSerializable)
};
FE_DECLARE_CLASS_BOTTOM(FeUiElement)