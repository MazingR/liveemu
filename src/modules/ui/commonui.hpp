#include <rendering/effect.hpp>
#include <common/serializable.hpp>
#include <common/maths.hpp>
#include <uielement.hpp>

class FeUiPanel : public FeUiElement
{
public:
#define FeUiPanel_Properties(_d)		\
	_d(FeTArray<FeUiElement>, Children)		\

	FE_DECLARE_CLASS_BODY(FeUiPanel_Properties, FeUiPanel, FeUiElement)
};
FE_DECLARE_CLASS_BOTTOM(FeUiPanel)

class FeScriptFile : public FeSerializable
{
public:
#define FeScriptFile_Properties(_d)				\
	_d(FeTArray<FeUiPanel>,			Panels)		\
	_d(FeTArray<FeRenderEffect>,	Effects)	\

	FE_DECLARE_CLASS_BODY(FeScriptFile_Properties, FeUiElement, FeSerializable)
};
FE_DECLARE_CLASS_BOTTOM(FeScriptFile)