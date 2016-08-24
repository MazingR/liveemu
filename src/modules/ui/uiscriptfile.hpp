#pragma once

#include <rendering/effect.hpp>
#include <common/serializable.hpp>
#include <common/maths.hpp>
#include <uielement.hpp>

class FeUiFont : public FeSerializable
{
public:
	FeUiFont()
	{
		Size = 32;
		Space = 16;
		Interval = 1;
		LineSpace = 2;
	}
#define props(_d)	\
	_d(FeString, Name)			\
	_d(FePath, TrueTypeFile)	\
	_d(uint32, Size)			\
	_d(uint32, Space)			\
	_d(uint32, Interval)		\
	_d(uint32, LineSpace)		\

	FE_DECLARE_CLASS_BODY(props, FeUiFont, FeSerializable)
#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeUiFont)


class FeScriptFile : public FeSerializable
{
public:
#define props(_d)				\
	_d(FeNTArray<FeTPtr<FeUiPanel>>,	Panels)		\
	_d(FeNTArray<FeTPtr<FeUiTemplate>>,	Templates)	\
	_d(FeNTArray<FeRenderEffect>,		Effects)	\
	_d(FeNTArray<FeUiFont>,				Fonts)		\

	FE_DECLARE_CLASS_BODY(props, FeScriptFile, FeSerializable)
#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeScriptFile)

class FeScrapperConfigFile : public FeSerializable
{
public:
#define props(_d)								\
	_d(bool,				Enable)				\
	_d(bool,				EnableFileWatch)	\
	_d(bool,				ShowLocalImports)	\
	_d(bool,				UseTwinPlatforms)	\

	FE_DECLARE_CLASS_BODY(props, FeScrapperConfigFile, FeSerializable)
#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeScrapperConfigFile)

class FeConfigFile : public FeSerializable
{
public:
#define props(_d)				\
	_d(FeString,				StartPanel)	\
	_d(FeScrapperConfigFile,	Scrapper)	\

	FE_DECLARE_CLASS_BODY(props, FeConfigFile, FeSerializable)
#undef props
};
FE_DECLARE_CLASS_BOTTOM(FeConfigFile)
