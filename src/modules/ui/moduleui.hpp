#pragma once

#include <common/common.hpp>
#include <common/tarray.hpp>
#include <common/application.hpp>
#include <common/filesystem.hpp>

#include <rendering/commonrenderer.hpp>

#include <commonui.hpp>
#include <uielement.hpp>
#include <uiscriptfile.hpp>

struct FeUiElementTraversalNode
{
	typedef FeTArray<FeUiElementTraversalNode*> TraversalArray;

	FeUiElement*				Current;
	FeUiElement*				Parent;
	FeResourceId				FontResource;
	bool						IsCulled;
	TraversalArray				Children;
	FeRenderGeometryInstance*	GeometryInstance;
	FeTransform					TraversalTransform;

	bool IsRoot() { return Parent == nullptr; }

	FeUiElementTraversalNode() :
		Parent(nullptr),
		Current(nullptr),
		GeometryInstance(nullptr)
	{}
};

struct FeUiDefferedApplyBinding
{
	FeString					SourceData;
	FeUiElementTraversalNode*	TraversalNode;
	const FeUiBinding*			BindingTarget;
	FeETargetPropertyType::Type TargetPropertyType;
};

struct FeUiElementTraversalList
{
	FeTArray<FeUiElementTraversalNode*> Nodes;

	FeUiElementTraversalNode* AddNode();

	FeUiElementTraversalList();
	~FeUiElementTraversalList();
};

struct FeUiRootPanel
{
	FeUiElementTraversalList			TraversalList;
	FeRenderBatch						RenderBatch;
	FeTArray<FeUiDefferedApplyBinding>	DefferedApplyBindingData;
	FeUiPanel*							Panel;

	FeUiRootPanel();
	~FeUiRootPanel();
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

private:
	uint32 LoadUnitTest(uint32 iTest);
	uint32 UpdateUnitTest(uint32 iTest, const FeDt& fDt);
	void TraverseElements(FeUiRootPanel& RootPanel);
	FeString FetchBindingSourceData(const FeUiBinding& binding);
	
	uint32 ApplyBindingToTargetProperty(FeUiRootPanel& RootPanel, FeUiElementTraversalNode* node, const FeString& sourceData, const FeUiBinding& targetBinding, FeETargetPropertyType::Type type);
	FeETargetPropertyType::Type GetTargetPropertyType(const FeUiBinding& targetBinding);
	void ComputeRenderingInstances();

	void ApplyBindingByType(FeETargetPropertyType::Type type);
	uint32 GenerateTextRenderingNodes(FeUiRootPanel& RootPanel, FeUiElementTraversalNode* node, const FeString& sourceData);
private:
	FeNTArray<FeScriptFile>			ScriptFiles;
	FeNTArray<FeUiRootPanel>		RootPanels;
	std::map<uint32, FeUiTemplate*>	Templates;
};