#include <uielement.hpp>

bool FeUiElement::HasState(uint32 states)
{
	return (Transient.States & states) != 0;
}
void FeUiElement::SetState(bool bVal, FeEUiElementState::Type state)
{
	uint32 iVal = (uint32)state;
	if (bVal)
		Transient.States |= iVal;
	else
		Transient.States &= ~iVal;
}
void FeUiElement::PreTraversal(FeUiRootPanel& RootPanel, FeUiElementTraversalNode* pNode)
{
}
void FeUiElement::PostTraversal(FeUiRootPanel& RootPanel, FeUiElementTraversalNode* pNode)
{
	Transient.States = FeEUiElementState::Visible | FeEUiElementState::Enabled;
	Transient.Transform = Transform;
}
void FeUiElement::PreApplyBindingByType(FeUiRootPanel& RootPanel, FeUiElementTraversalNode* pNode)
{
}
void FeUiElement::PostInitialize(FeUiRootPanel& RootPanel, FeUiElementTraversalNode* pNode)
{
}

void FeUiContainer::PreTraversal(FeUiRootPanel& RootPanel, FeUiElementTraversalNode* pNode)
{
	Super::PreTraversal(RootPanel, pNode);
}
void FeUiContainer::PostTraversal(FeUiRootPanel& RootPanel, FeUiElementTraversalNode* pNode)
{
	Super::PostTraversal(RootPanel, pNode);
}
void FeUiContainer::PreApplyBindingByType(FeUiRootPanel& RootPanel, FeUiElementTraversalNode* pNode)
{
	Super::PreApplyBindingByType(RootPanel, pNode);

}
void FeUiContainer::PostInitialize(FeUiRootPanel& RootPanel, FeUiElementTraversalNode* pNode)
{
	Super::PostInitialize(RootPanel, pNode);
}