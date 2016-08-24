#pragma once

#include <rendering/effect.hpp>
#include <common/serializable.hpp>
#include <common/maths.hpp>
#include <commonui.hpp>

class FeUiElement;
class FeUiPanel;

struct FeUiRootPanel;
struct FeUiElementTraversalNode;

struct FeUiElementProperties
{
	FeTransform				Transform;
	FeTArray<FeUiElement*>	Children;
	uint32					States;
};

class FeUiElement : public FeSerializable
{
public:
	bool HasState(uint32 states);
	void SetState(bool bVal, FeEUiElementState::Type state);

	inline bool IsVisible()		{ return HasState(FeEUiElementState::Visible	); }
	inline bool IsEnabled()		{ return HasState(FeEUiElementState::Enabled	); }
	inline bool IsCollapsed()	{ return HasState(FeEUiElementState::Collapsed	); }
	inline bool IsFocused()		{ return HasState(FeEUiElementState::Focused	); }
	inline bool IsSelected()	{ return HasState(FeEUiElementState::Selected	); }

	inline void SetVisible(bool bVal)	{ SetState(bVal, FeEUiElementState::Visible);	 OnVisibleChanged(bVal);	}
	inline void SetEnabled(bool bVal)	{ SetState(bVal, FeEUiElementState::Enabled);	 OnEnabledChanged(bVal);	}
	inline void SetCollapsed(bool bVal)	{ SetState(bVal, FeEUiElementState::Collapsed);  OnCollapsedChanged(bVal);	}
	inline void SetFocused(bool bVal)	{ SetState(bVal, FeEUiElementState::Focused);	 OnFocusedChanged(bVal);	}
	inline void SetSelected(bool bVal)	{ SetState(bVal, FeEUiElementState::Selected);	 OnSelectedChanged(bVal);	}

	virtual void OnVisibleChanged(bool bVal)	{}
	virtual void OnEnabledChanged(bool bVal)	{}
	virtual void OnCollapsedChanged(bool bVal)	{}
	virtual void OnFocusedChanged(bool bVal)	{}
	virtual void OnSelectedChanged(bool bVal)	{}

	#define FeUiElement_Properties(_d)						\
		_d(FeTransform,						Transform)		\
		_d(FeString,						Name)			\
		_d(FeNTArray<FeTPtr<FeUiElement>>,	Children)		\
		_d(FeString,						Effect)			\
		_d(FeString,						FontEffect)		\
		_d(FeString,						Font)			\
		_d(FeNTArray<FeUiDataBinding>,		Bindings)		\

	FE_DECLARE_CLASS_BODY(FeUiElement_Properties, FeUiElement, FeSerializable)

	virtual void ComputeChilren(FeUiRootPanel& RootPanel){}

	virtual void PreTraversal(FeUiRootPanel& RootPanel, FeUiElementTraversalNode* pNode);
	virtual void PostTraversal(FeUiRootPanel& RootPanel, FeUiElementTraversalNode* pNode);
	virtual void PreApplyBindingByType(FeUiRootPanel& RootPanel, FeUiElementTraversalNode* pNode);
	virtual void PostInitialize(FeUiRootPanel& RootPanel, FeUiElementTraversalNode* pNode);

	FeUiElementProperties Transient;
};
FE_DECLARE_CLASS_BOTTOM(FeUiElement)

// ----------------------------------------------------------------------------------------------------------------------------------------------------------

class FeUiTemplate : public FeUiElement
{
public:
#define props(_d)				\

	FE_DECLARE_CLASS_BODY(props, FeUiTemplate, FeUiElement)
#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeUiTemplate)

class FeUiPanel : public FeUiElement
{
public:
#define props(_d)				\

	FE_DECLARE_CLASS_BODY(props, FeUiPanel, FeUiElement)
#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeUiPanel)

class FeUiContainer : public FeUiElement
{
public:
#define props(_d)				\
	_d(FeString, Template)		\

	virtual void PreTraversal(FeUiRootPanel& RootPanel, FeUiElementTraversalNode* pNode) override;
	virtual void PostTraversal(FeUiRootPanel& RootPanel, FeUiElementTraversalNode* pNode) override;
	virtual void PreApplyBindingByType(FeUiRootPanel& RootPanel, FeUiElementTraversalNode* pNode) override;
	virtual void PostInitialize(FeUiRootPanel& RootPanel, FeUiElementTraversalNode* pNode) override;

	FE_DECLARE_CLASS_BODY(props, FeUiContainer, FeUiElement)
#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeUiContainer)

class FeUiContainerList : public FeUiContainer
{
public:
#define props(_d)				\

	FE_DECLARE_CLASS_BODY(props, FeUiContainerList, FeUiContainer)
#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeUiContainerList)
