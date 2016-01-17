#include <uielement.hpp>

bool FeUiElement::HasState(FeEUiElementState::Type state)
{
	return (this->State & state) != 0;
}
bool FeUiElement::IsSelected()
{
	return HasState(FeEUiElementState::Selected);
}
bool FeUiElement::IsVisible()
{
	return HasState(FeEUiElementState::Visible);
}
