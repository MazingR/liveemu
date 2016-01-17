#include <rendering/effect.hpp>
#include <common/serializable.hpp>
#include <common/maths.hpp>

#define FeEUiElementState_Values(_d)		\
		_d(FeEUiElementState, Visible)		\
		_d(FeEUiElementState, Collapsed)	\
		_d(FeEUiElementState, Selected)		\

FE_DECLARE_ENUM(FeEUiElementState, FeEUiElementState_Values)

class FeUiElement : public FeSerializable
{
public:
	bool HasState(FeEUiElementState::Type state);
	bool IsVisible();
	bool IsSelected();

	#define FeUiElement_Properties(_d)					\
		_d(FeTransform,						Transform)	\
		_d(FeEUiElementState::Type,			State)		\
		_d(FeTArray<FeTPtr<FeUiElement>>,	Children)	\
		_d(FeString,						Effect)		\

	FE_DECLARE_CLASS_BODY(FeUiElement_Properties, FeUiElement, FeSerializable)
};
FE_DECLARE_CLASS_BOTTOM(FeUiElement)