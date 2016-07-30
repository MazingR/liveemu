#pragma once

#include <common/common.hpp>
#include <common/tarray.hpp>
#include <common/application.hpp>
#include <common/filesystem.hpp>

#include <rendering/commonrenderer.hpp>
#include <commonui.hpp>

struct FeRenderBatch;

struct FeUiElementTraversalNode
{
	FeUiElement*				Current;
	FeUiElement*				Parent;

	FeResourceId				FontResource;
	bool						IsCulled;

	FeTArray<FeUiElementTraversalNode*>	Children;

	FeRenderGeometryInstance*			GeometryInstance;

	FeUiElementTraversalNode() : Parent(nullptr), Current(nullptr), GeometryInstance(nullptr) {}
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

private:
	uint32 LoadUnitTest(uint32 iTest);
	uint32 UpdateUnitTest(uint32 iTest, const FeDt& fDt);
	void TraverseElements(FeScriptFile& script, FeUiElementTraversalList& traversal);
	FeString FetchBindingSourceData(const FeUiBinding& binding);
	uint32 ApplyBindingToTargetProperty(FeUiElementTraversalNode& node, const FeString& sourceData, const FeUiBinding& targetBinding, FeETargetPropertyType::Type type);
	FeETargetPropertyType::Type GetTargetPropertyType(const FeUiBinding& targetBinding);
	void ComputeRenderingInstances();
	void ApplyBindingByType(FeETargetPropertyType::Type type);
	uint32 GenerateTextRenderingNodes(FeUiElementTraversalNode& node, const FeString& sourceData);
private:
	FeNTArray<FeScriptFile>				ScriptFiles;
	FeTArray<FeRenderBatch>				RenderBatches;
	
	FeTArray<FeUiPanel*>			Panels;
	FeTArray<FeRenderEffect*>		Effects;
	FeTArray<FeUiFont*>				Fonts;

	FeUiElementTraversalList		TraversalList;

	FeTArray<FeUiDefferedApplyBinding> DefferedApplyBindingData;
};