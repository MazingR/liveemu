#include <rendering/effect.hpp>
#include <common/serializable.hpp>
#include <common/maths.hpp>
#include <uielement.hpp>

class FeUiPanel : public FeUiElement
{
public:
#define FeUiPanel_Properties(_d)		\
	_d(FeTArray<FeUiElement>, Children)	\

	FE_DECLARE_CLASS_BODY(FeUiPanel_Properties, FeUiPanel, FeUiElement)
};
FE_DECLARE_CLASS_BOTTOM(FeUiPanel)

class FeUiFont : public FeSerializable
{
public:
#define FeUiFont_Properties(_d)	\
	_d(FeString, Name)			\
	_d(FePath, TrueTypeFile)	\
	_d(uint32, Size)			\

	FE_DECLARE_CLASS_BODY(FeUiFont_Properties, FeUiFont, FeSerializable)
};
FE_DECLARE_CLASS_BOTTOM(FeUiFont)

class FeScriptFile : public FeSerializable
{
public:
#define FeScriptFile_Properties(_d)				\
	_d(FeTArray<FeUiPanel>,			Panels)		\
	_d(FeTArray<FeRenderEffect>,	Effects)	\
	_d(FeTArray<FeUiFont>,			Fonts)		\

	FE_DECLARE_CLASS_BODY(FeScriptFile_Properties, FeUiElement, FeSerializable)
};
FE_DECLARE_CLASS_BOTTOM(FeScriptFile)