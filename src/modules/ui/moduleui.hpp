#pragma once

#include <common/common.hpp>
#include <common/tarray.hpp>
#include <common/application.hpp>
#include <common/filesystem.hpp>

#include <rendering/commonrenderer.hpp>
#include <commonui.hpp>

struct FeUiRenderingInstance
{
	FeUiElement*						Owner;
	FeRenderGeometryInstance			Geometry;
};

struct FeUiElementTraversalNode
{
	FeUiElement*						Current;
	FeUiElement*						Parent;
	FeTArray<FeUiElementTraversalNode*>	Children;
	FeUiRenderingInstance*				RenderInstance;

	FeUiElementTraversalNode() : Parent(NULL) {}
};

struct FeUiElementTraversalList
{
	FeTArray<FeUiElementTraversalNode> Nodes;
};

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

	uint32 ReloadScripts();
	uint32 LoadUnitTest(uint32 iTest);
	uint32 UpdateUnitTest(uint32 iTest, const FeDt& fDt);
	void TraverseElements(FeScriptFile& script, FeUiElementTraversalList& traversal);
	FeString FetchBindingSourceData(const FeUiBinding& binding);
	void ApplyBindingToTargetProperty(FeUiElementTraversalNode& node, const FeString& sourceData, const FeUiBinding& targetBinding);

private:
	FeTArray<FeScriptFile>			ScriptFiles;
	FeTArray<FeUiRenderingInstance> RenderingInstances;
	FeTArray<FeUiPanel*>			Panels;
	FeUiElementTraversalList		TraversalList;
};

//
//class FeModuleFontsHandler : public FeModule
//{
//public:
//	virtual uint32 Load(const FeModuleInit*) override;
//	virtual uint32 Unload() override;
//	virtual uint32 Update(const FeDt& fDt) override;
//
//	uint32 LoadFontFromFile(const FePath& path);
//
//private:
//
//	static void FontLoadingCallback(FeRenderTexture* pTexture, void* pUserData);
//
//	FT_LibraryRec_*	FtLibrary;
//	FeTArray<FeFontData> Fonts;
//};
//
//struct FeFontLoadingData
//{
//	FePath					FontPath;
//	FeModuleFontsHandler*	This;
//};
